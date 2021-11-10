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
        ImGui::NewFrame();
    }

    void
    mvEndFrame()
    {

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

    void
    mvRenderMesh(mvAssetManager& am, mvMesh& mesh, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection)
    {
        mv_local_persist VkDeviceSize offsets = { 0 };

        if (mesh.phongMaterialID == -1)
        {
            mesh.phongMaterialID = mvGetPhongMaterialAsset(&am, {}, "vs_shader.vert.spv", "ps_shader.frag.spv");

            mvPipelineSpec spec{};
            spec.backfaceCulling = true;
            spec.depthTest = true;
            spec.wireFrame = false;
            spec.depthWrite = true;
            spec.vertexShader = "vs_shader.vert.spv";
            spec.pixelShader = "ps_shader.frag.spv";
            spec.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            am.phongMaterials[mesh.phongMaterialID].material.pipeline = mvGetPipelineAsset(&am, spec);
        }

        mvAssetID materialID = mesh.phongMaterialID;
        mvAssetID pipelineID = am.phongMaterials[materialID].material.pipeline;
        mvAssetID layoutID = am.pipelines[pipelineID].pipeline.pipelineLayout;

        VkCommandBuffer commandBuffer = GContext->graphics.commandBuffers[GContext->graphics.currentImageIndex];
        VkPipeline pipeline = am.pipelines[pipelineID].pipeline.pipeline;
        VkPipelineLayout pipelineLayout = am.pipelineLayouts[layoutID].layout.pipelineLayout;
        VkDescriptorSet descriptorSet = am.phongMaterials[materialID].material.descriptorSets[GContext->graphics.currentImageIndex];
        VkBuffer indexBuffer = am.buffers[mesh.indexBuffer].buffer.buffer;
        VkBuffer vertexBuffer = am.buffers[mesh.vertexBuffer].buffer.buffer;

        if(mesh.diffuseTexture != -1) 
            mvUpdateMaterialDescriptors(am, am.phongMaterials[mesh.phongMaterialID].material, mesh.diffuseTexture, mesh.normalTexture, mesh.specularTexture);
        vkCmdBindDescriptorSets(mvGetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &descriptorSet, 0, nullptr);
        vkCmdBindIndexBuffer(mvGetCurrentCommandBuffer(), indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindVertexBuffers(mvGetCurrentCommandBuffer(), 0, 1, &vertexBuffer, &offsets);
        vkCmdBindPipeline(mvGetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        mvTransforms transforms;
        transforms.model = accumulatedTransform;
        transforms.modelView = camera * transforms.model;
        transforms.modelViewProjection = projection * transforms.modelView;

        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvTransforms), &transforms);

        vkCmdDrawIndexed(mvGetCurrentCommandBuffer(), am.buffers[mesh.indexBuffer].buffer.count, 1, 0, 0, 0);
    }

    void
    mvBeginPass(VkCommandBuffer commandBuffer, mvPass& pass, b8 clear)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("failed to begin recording command buffer!");

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

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer!");
    }

    mv_internal void
    mvRenderNode(mvAssetManager& am, mvNode& node, mvMat4 accumulatedTransform, mvMat4 cam, mvMat4 proj)
    {

        if (node.mesh > -1)
            mvRenderMesh(am, am.meshes[node.mesh].mesh, accumulatedTransform * node.matrix, cam, proj);

        for (u32 i = 0; i < node.childCount; i++)
        {
            mvRenderNode(am, am.nodes[node.children[i]].node, accumulatedTransform * node.matrix, cam, proj);
        }
    }

    void
    mvRenderScene(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj)
    {
        for (u32 i = 0; i < scene.nodeCount; i++)
        {
            mvNode& rootNode = am.nodes[scene.nodes[i]].node;

            if (rootNode.mesh > -1)
                mvRenderMesh(am, am.meshes[rootNode.mesh].mesh, rootNode.matrix, cam, proj);

            for (u32 j = 0; j < rootNode.childCount; j++)
            {
                mvRenderNode(am, am.nodes[rootNode.children[j]].node, rootNode.matrix, cam, proj);
            }
        }
    }

}