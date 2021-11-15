#include "mvRenderer.h"
#include <stdexcept>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include "mvAssetManager.h"

namespace Renderer {

    void
    mvBeginFrame()
    {
        vkWaitForFences(mvGetLogicalDevice(), 1, &GContext->graphics.inFlightFences[GContext->graphics.currentFrame], VK_TRUE, UINT64_MAX);

        VkResult result = vkAcquireNextImageKHR(mvGetLogicalDevice(), GContext->graphics.swapChain, UINT64_MAX, GContext->graphics.imageAvailableSemaphores[GContext->graphics.currentFrame],
            VK_NULL_HANDLE, &GContext->graphics.currentImageIndex);

        if (GContext->graphics.imagesInFlight[GContext->graphics.currentImageIndex] != VK_NULL_HANDLE)
            vkWaitForFences(mvGetLogicalDevice(), 1, &GContext->graphics.imagesInFlight[GContext->graphics.currentImageIndex], VK_TRUE, UINT64_MAX);

        // just in case the acquired image is out of order
        GContext->graphics.imagesInFlight[GContext->graphics.currentImageIndex] = GContext->graphics.inFlightFences[GContext->graphics.currentFrame];

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(mvGetCurrentCommandBuffer(), &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("failed to begin recording command buffer!");
    }

    void
    mvEndFrame()
    {

        if (vkEndCommandBuffer(mvGetCurrentCommandBuffer()) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer!");

        VkSemaphore waitSemaphores[] = { GContext->graphics.imageAvailableSemaphores[GContext->graphics.currentFrame] };
        VkSemaphore signalSemaphores[] = { GContext->graphics.renderFinishedSemaphores[GContext->graphics.currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &GContext->graphics.commandBuffers[GContext->graphics.currentImageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(mvGetLogicalDevice(), 1, &GContext->graphics.inFlightFences[GContext->graphics.currentFrame]);

        if (vkQueueSubmit(GContext->graphics.graphicsQueue, 1, &submitInfo, GContext->graphics.inFlightFences[GContext->graphics.currentFrame]) != VK_SUCCESS)
            throw std::runtime_error("failed to submit draw command buffer!");
    }

    void mvUpdateDescriptors(mvAssetManager& am)
    {
        for (int i = 0; i < am.meshCount; i++)
        {
            mvMesh& mesh = am.meshes[i].asset;
            mvUpdateMaterialDescriptors(am, am.phongMaterials[mesh.phongMaterialID].asset, mesh.diffuseTexture, mesh.normalTexture, mesh.specularTexture);
        }
    }

    void
    mvRenderMesh(mvAssetManager& am, mvMesh& mesh, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection)
    {
        mv_local_persist VkDeviceSize offsets = { 0 };

        VkCommandBuffer commandBuffer = GContext->graphics.commandBuffers[GContext->graphics.currentImageIndex];
        VkBuffer indexBuffer = am.buffers[mesh.indexBuffer].asset.buffer;
        VkBuffer vertexBuffer = am.buffers[mesh.vertexBuffer].asset.buffer;

        mvBindDescriptorSet(am, am.phongMaterials[mesh.phongMaterialID].asset.descriptorSets, 1);
        vkCmdBindIndexBuffer(mvGetCurrentCommandBuffer(), indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindVertexBuffers(mvGetCurrentCommandBuffer(), 0, 1, &vertexBuffer, &offsets);

        mvTransforms transforms;
        transforms.model = accumulatedTransform;
        transforms.modelView = camera * transforms.model;
        transforms.modelViewProjection = projection * transforms.modelView;

        VkPipelineLayout mainPipelineLayout = mvGetRawPipelineLayoutAsset(&am, "main_pass");
        vkCmdPushConstants(commandBuffer, mainPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvTransforms), &transforms);

        vkCmdDrawIndexed(mvGetCurrentCommandBuffer(), am.buffers[mesh.indexBuffer].asset.count, 1, 0, 0, 0);
    }

    void
    mvBeginPass(VkCommandBuffer commandBuffer, mvPass& pass, b8 clear)
    {

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = pass.renderPass;
        renderPassInfo.framebuffer = pass.frameBuffers[GContext->graphics.currentImageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = pass.extent;

        VkClearValue clearValues[2];
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };
        if (clear)
        {
            renderPassInfo.clearValueCount = 2;
            renderPassInfo.pClearValues = clearValues;
        }

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdSetViewport(commandBuffer, 0, 1, &pass.viewport);

        VkRect2D scissor{};
        scissor.extent = pass.extent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void
    mvEndPass(VkCommandBuffer commandBuffer)
    {
        vkCmdEndRenderPass(commandBuffer);
    }

    mv_internal void
    mvRenderNode(mvAssetManager& am, mvNode& node, mvMat4 accumulatedTransform, mvMat4 cam, mvMat4 proj)
    {

        if (node.mesh > -1)
            mvRenderMesh(am, am.meshes[node.mesh].asset, accumulatedTransform * node.matrix, cam, proj);

        for (u32 i = 0; i < node.childCount; i++)
        {
            mvRenderNode(am, am.nodes[node.children[i]].asset, accumulatedTransform * node.matrix, cam, proj);
        }
    }

    void
    mvRenderScene(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj)
    {

        for (u32 i = 0; i < scene.nodeCount; i++)
        {
            mvNode& rootNode = am.nodes[scene.nodes[i]].asset;

            if (rootNode.mesh > -1)
                mvRenderMesh(am, am.meshes[rootNode.mesh].asset, rootNode.matrix, cam, proj);

            for (u32 j = 0; j < rootNode.childCount; j++)
            {
                mvRenderNode(am, am.nodes[rootNode.children[j]].asset, rootNode.matrix, cam, proj);
            }
        }
    }

}