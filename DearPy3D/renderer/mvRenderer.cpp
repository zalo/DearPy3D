#include "mvRenderer.h"
#include <stdexcept>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include "mvAssetManager.h"
#include "mvGraphics.h"
#include "mvSkybox.h"

namespace Renderer {

    void
    begin_frame(mvGraphics& graphics)
    {
        MV_VULKAN(vkWaitForFences(graphics.logicalDevice, 1, &graphics.inFlightFences[graphics.currentFrame], VK_TRUE, UINT64_MAX));

        MV_VULKAN(vkAcquireNextImageKHR(graphics.logicalDevice, graphics.swapChain, UINT64_MAX, graphics.imageAvailableSemaphores[graphics.currentFrame],
            VK_NULL_HANDLE, &graphics.currentImageIndex));

        if (graphics.imagesInFlight[graphics.currentImageIndex] != VK_NULL_HANDLE)
            MV_VULKAN(vkWaitForFences(graphics.logicalDevice, 1, &graphics.imagesInFlight[graphics.currentImageIndex], VK_TRUE, UINT64_MAX));

        // just in case the acquired image is out of order
        graphics.imagesInFlight[graphics.currentImageIndex] = graphics.inFlightFences[graphics.currentFrame];

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        MV_VULKAN(vkBeginCommandBuffer(get_current_command_buffer(graphics), &beginInfo));
    }

    void
    end_frame(mvGraphics& graphics)
    {

        MV_VULKAN(vkEndCommandBuffer(get_current_command_buffer(graphics)));

        VkSemaphore waitSemaphores[] = { graphics.imageAvailableSemaphores[graphics.currentFrame] };
        VkSemaphore signalSemaphores[] = { graphics.renderFinishedSemaphores[graphics.currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &graphics.commandBuffers[graphics.currentImageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        MV_VULKAN(vkResetFences(graphics.logicalDevice, 1, &graphics.inFlightFences[graphics.currentFrame]));
        MV_VULKAN(vkQueueSubmit(graphics.graphicsQueue, 1, &submitInfo, graphics.inFlightFences[graphics.currentFrame]));
    }

    void 
    update_descriptors(mvGraphics& graphics, mvAssetManager& am, mvMaterialManager& mManager)
    {
        for (int i = 0; i < am.meshCount; i++)
        {
            mvMesh& mesh = am.meshes[i].asset;
            update_material_descriptors(graphics, am, mManager.materials[mesh.phongMaterialID], mesh.diffuseTexture, mesh.normalTexture, mesh.specularTexture);
        }
    }

    void
    render_mesh(mvGraphics& graphics, mvDescriptorSet& descriptorSet, VkPipelineLayout pipelineLayout, mvMesh& mesh, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection)
    {
        mv_local_persist VkDeviceSize offsets = { 0 };

        VkCommandBuffer commandBuffer = graphics.commandBuffers[graphics.currentImageIndex];
        bind_descriptor_set(graphics, descriptorSet, 1);
        vkCmdBindIndexBuffer(get_current_command_buffer(graphics), mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindVertexBuffers(get_current_command_buffer(graphics), 0, 1, &mesh.vertexBuffer.buffer, &offsets);

        mvTransforms transforms;
        transforms.model = accumulatedTransform;
        transforms.modelView = camera * transforms.model;
        transforms.modelViewProjection = projection * transforms.modelView;

        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvTransforms), &transforms);
        vkCmdDrawIndexed(get_current_command_buffer(graphics), mesh.indexBuffer.specification.count, 1, 0, 0, 0);
    }

    void
    render_mesh_omni_shadow(mvGraphics& graphics, VkPipelineLayout pipelineLayout, mvMesh& mesh, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection, mvVec4 lightPos)
    {
        mv_local_persist VkDeviceSize offsets = { 0 };

        VkCommandBuffer commandBuffer = graphics.commandBuffers[graphics.currentImageIndex];
        vkCmdBindIndexBuffer(get_current_command_buffer(graphics), mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindVertexBuffers(get_current_command_buffer(graphics), 0, 1, &mesh.vertexBuffer.buffer, &offsets);

        mvTransforms transforms;
        transforms.model = accumulatedTransform;
        transforms.modelView = camera * transforms.model;
        transforms.modelViewProjection = projection * transforms.modelView;

        struct tempstruct
        {
            mvMat4 mvp;
            mvVec4 lightPos;
        };

        tempstruct push = { transforms.modelViewProjection, lightPos };
        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(tempstruct), &push);
        vkCmdDrawIndexed(get_current_command_buffer(graphics), mesh.indexBuffer.specification.count, 1, 0, 0, 0);
    }

    void
    render_mesh_shadow(mvGraphics& graphics, VkPipelineLayout pipelineLayout, mvMesh& mesh, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection)
    {
        mv_local_persist VkDeviceSize offsets = { 0 };

        VkCommandBuffer commandBuffer = graphics.commandBuffers[graphics.currentImageIndex];
        vkCmdBindIndexBuffer(get_current_command_buffer(graphics), mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindVertexBuffers(get_current_command_buffer(graphics), 0, 1, &mesh.vertexBuffer.buffer, &offsets);

        mvTransforms transforms;
        transforms.model = accumulatedTransform;
        transforms.modelView = camera * transforms.model;
        transforms.modelViewProjection = projection * transforms.modelView;

        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvTransforms), &transforms);
        vkCmdDrawIndexed(get_current_command_buffer(graphics), mesh.indexBuffer.specification.count, 1, 0, 0, 0);
    }

    void
    render_skybox(mvSkybox& skybox, mvGraphics& graphics, VkPipelineLayout pipelineLayout, mvMat4 cam, mvMat4 proj)
    {
        mv_local_persist VkDeviceSize offsets = { 0 };

        VkCommandBuffer commandBuffer = graphics.commandBuffers[graphics.currentImageIndex];
        vkCmdBindIndexBuffer(get_current_command_buffer(graphics), skybox.mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindVertexBuffers(get_current_command_buffer(graphics), 0, 1, &skybox.mesh.vertexBuffer.buffer, &offsets);

        mvTransforms transforms;
        transforms.model = mvMat4(1.0f);
        transforms.modelView = cam * transforms.model;
        transforms.modelViewProjection = proj * transforms.modelView;

        mvMat4 modelViewProjection = transforms.modelViewProjection;

        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvMat4), &modelViewProjection);
        vkCmdDrawIndexed(get_current_command_buffer(graphics), skybox.mesh.indexBuffer.specification.count, 1, 0, 0, 0);
    }

    void
    begin_pass(mvGraphics& graphics, mvPipelineManager& pmManager, VkCommandBuffer commandBuffer, mvPass& pass)
    {

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = pass.renderPass;
        renderPassInfo.framebuffer = pass.frameBuffers[graphics.currentImageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = pass.extent;

        VkClearValue clearValues[2];
        if (pass.specification.hasColor && pass.specification.hasDepth)
        {

            clearValues[0].color = *(VkClearColorValue*)pass.specification.clearColorValue;
            clearValues[1].depthStencil = { pass.specification.clearDepthValue, 0 };
            renderPassInfo.clearValueCount = 2;
            renderPassInfo.pClearValues = clearValues;
        }
        else if (pass.specification.hasColor)
        {

            clearValues[0].color = *(VkClearColorValue*)pass.specification.clearColorValue;
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = clearValues;
        }
        else if (pass.specification.hasDepth)
        {
            clearValues[0].depthStencil = { pass.specification.clearDepthValue, 0 };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = clearValues;
        }

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdSetViewport(commandBuffer, 0, 1, &pass.viewport);

        VkRect2D scissor{};
        scissor.extent = pass.extent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdSetDepthBias(
            get_current_command_buffer(graphics),
            pass.specification.depthBias,
            0.0f,
            pass.specification.slopeDepthBias);

        if (pass.specification.pipeline != MV_INVALID_ASSET_ID)
        {
            VkPipeline pipeline = pmManager.pipelines[pass.specification.pipeline].pipeline;
            vkCmdBindPipeline(get_current_command_buffer(graphics), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        }
    }

    void
    end_pass(VkCommandBuffer commandBuffer)
    {
        vkCmdEndRenderPass(commandBuffer);
    }

    mv_internal void
    mvRenderNode(mvGraphics& graphics, mvAssetManager& am, mvMaterialManager& mManager, VkPipelineLayout pipelineLayout, mvNode& node, mvMat4 accumulatedTransform, mvMat4 cam, mvMat4 proj)
    {
        
        if (node.mesh > -1)
        {
            mvDescriptorSet& descriptorSet = mManager.materials[am.meshes[node.mesh].asset.phongMaterialID].descriptorSet;
            render_mesh(graphics, descriptorSet, pipelineLayout, am.meshes[node.mesh].asset, accumulatedTransform * node.matrix, cam, proj);
        }

        for (u32 i = 0; i < node.childCount; i++)
        {
            mvRenderNode(graphics, am, mManager, pipelineLayout, am.nodes[node.children[i]].asset, accumulatedTransform * node.matrix, cam, proj);
        }
    }

    mv_internal void
    mvRenderNodeShadow(mvGraphics& graphics, mvAssetManager& am, VkPipelineLayout pipelineLayout, mvNode& node, mvMat4 accumulatedTransform, mvMat4 cam, mvMat4 proj)
    {

        if (node.mesh > -1)
            render_mesh_shadow(graphics, pipelineLayout, am.meshes[node.mesh].asset, accumulatedTransform * node.matrix, cam, proj);

        for (u32 i = 0; i < node.childCount; i++)
        {
            mvRenderNodeShadow(graphics, am, pipelineLayout, am.nodes[node.children[i]].asset, accumulatedTransform * node.matrix, cam, proj);
        }
    }

    mv_internal void
    mvRenderNodeOmniShadow(mvGraphics& graphics, mvAssetManager& am, VkPipelineLayout pipelineLayout, mvNode& node, mvMat4 accumulatedTransform, mvMat4 cam, mvMat4 proj, mvVec4 lightPos)
    {

        if (node.mesh > -1)
            render_mesh_omni_shadow(graphics, pipelineLayout, am.meshes[node.mesh].asset, accumulatedTransform * node.matrix, cam, proj, lightPos);

        for (u32 i = 0; i < node.childCount; i++)
        {
            mvRenderNodeOmniShadow(graphics, am, pipelineLayout, am.nodes[node.children[i]].asset, accumulatedTransform * node.matrix, cam, proj, lightPos);
        }
    }

    void
    render_scene(mvGraphics& graphics, mvAssetManager& am, mvMaterialManager& mManager, VkPipelineLayout pipelineLayout, mvScene& scene, mvMat4 cam, mvMat4 proj)
    {

        for (u32 i = 0; i < scene.nodeCount; i++)
        {
            mvNode& rootNode = am.nodes[scene.nodes[i]].asset;

            if (rootNode.mesh > -1)
            {
                mvDescriptorSet& descriptorSet = mManager.materials[am.meshes[rootNode.mesh].asset.phongMaterialID].descriptorSet;
                render_mesh(graphics, descriptorSet, pipelineLayout, am.meshes[rootNode.mesh].asset, rootNode.matrix, cam, proj);
            }

            for (u32 j = 0; j < rootNode.childCount; j++)
            {
                mvRenderNode(graphics, am, mManager, pipelineLayout, am.nodes[rootNode.children[j]].asset, rootNode.matrix, cam, proj);
            }
        }
    }

    void
    render_scene_shadows(mvGraphics& graphics, mvAssetManager& am, VkPipelineLayout pipelineLayout, mvScene& scene, mvMat4 cam, mvMat4 proj)
    {

        for (u32 i = 0; i < scene.nodeCount; i++)
        {
            mvNode& rootNode = am.nodes[scene.nodes[i]].asset;

            if (rootNode.mesh > -1)
            {
                render_mesh_shadow(graphics, pipelineLayout, am.meshes[rootNode.mesh].asset, rootNode.matrix, cam, proj);
            }

            for (u32 j = 0; j < rootNode.childCount; j++)
            {
                mvRenderNodeShadow(graphics, am, pipelineLayout, am.nodes[rootNode.children[j]].asset, rootNode.matrix, cam, proj);
            }
        }
    }

    void
    render_scene_omni_shadows(mvGraphics& graphics, mvAssetManager& am, VkPipelineLayout pipelineLayout, mvScene& scene, mvMat4 cam, mvMat4 proj, mvVec4 lightPos)
    {

        for (u32 i = 0; i < scene.nodeCount; i++)
        {
            mvNode& rootNode = am.nodes[scene.nodes[i]].asset;

            if (rootNode.mesh > -1)
                render_mesh_omni_shadow(graphics, pipelineLayout, am.meshes[rootNode.mesh].asset, rootNode.matrix, cam, proj, lightPos);

            for (u32 j = 0; j < rootNode.childCount; j++)
            {
                mvRenderNodeOmniShadow(graphics, am, pipelineLayout, am.nodes[rootNode.children[j]].asset, rootNode.matrix, cam, proj, lightPos);
            }
        }
    }

    mvPass
    create_omni_shadow_pass(mvGraphics& graphics, mvAssetManager& am, mvPassSpecification specification)
    {

        mvPass pass{};
        pass.specification = specification;

        VkAttachmentDescription attchmentDescriptions[2];

        // Color attachment
        attchmentDescriptions[0].flags = 0;
        attchmentDescriptions[0].format = specification.colorFormat;
        attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Depth attachment
        attchmentDescriptions[1].flags = 0;
        attchmentDescriptions[1].format = specification.depthFormat;
        attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;
        subpassDescription.pDepthStencilAttachment = &depthReference;

        // Create the actual renderpass
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 2;
        renderPassInfo.pAttachments = attchmentDescriptions;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescription;

        MV_VULKAN(vkCreateRenderPass(graphics.logicalDevice, &renderPassInfo, nullptr, &pass.renderPass));

        pass.colorTextures.resize(graphics.swapChainImageViews.size());
        pass.depthTextures.resize(graphics.swapChainImageViews.size());
        pass.frameBuffers.resize(graphics.swapChainImageViews.size());
        for (size_t i = 0; i < graphics.swapChainImageViews.size(); i++)
        {
            pass.colorTextures[i] = create_texture(graphics,
                specification.width, specification.height, specification.colorFormat,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);

            VkCommandBuffer commandBuffer = begin_command_buffer(graphics);
            transition_image_layout(commandBuffer, pass.colorTextures[i].textureImage,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            );
            submit_command_buffer(graphics, commandBuffer);

            pass.depthTextures[i] = create_texture(graphics,
                specification.width, specification.height, specification.depthFormat,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

            VkCommandBuffer commandBuffer2 = begin_command_buffer(graphics);
            transition_image_layout(commandBuffer2, pass.depthTextures[i].textureImage,
                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            );

            submit_command_buffer(graphics, commandBuffer2);

            VkImageView imageViews[] = { pass.colorTextures[i].imageInfo.imageView, pass.depthTextures[i].imageInfo.imageView };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = pass.renderPass;
            framebufferInfo.attachmentCount = 2;
            framebufferInfo.pAttachments = imageViews;
            framebufferInfo.width = specification.width;
            framebufferInfo.height = specification.height;
            framebufferInfo.layers = 1;
            MV_VULKAN(vkCreateFramebuffer(graphics.logicalDevice, &framebufferInfo, nullptr, &pass.frameBuffers[i]));
        }

        pass.pipelineSpec.width = pass.specification.width;
        pass.pipelineSpec.height = pass.specification.height;
        pass.pipelineSpec.renderPass = pass.renderPass;

        pass.viewport.x = 0.0f;
        pass.viewport.y = pass.specification.height;
        pass.viewport.width = pass.specification.width;
        pass.viewport.height = -pass.specification.height;
        pass.viewport.minDepth = 0.0f;
        pass.viewport.maxDepth = 1.0f;
        pass.extent.width = (u32)pass.specification.width;
        pass.extent.height = (u32)pass.viewport.y;

        return pass;
    }

    mvPass
    create_primary_pass(mvGraphics& graphics, mvAssetManager& am, mvPassSpecification specification)
    {

        mvPass pass{};
        pass.specification = specification;

        VkAttachmentDescription attchmentDescriptions[2];

        // Color attachment
        attchmentDescriptions[0].flags = 0;
        attchmentDescriptions[0].format = specification.colorFormat;
        attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Depth attachment
        attchmentDescriptions[1].flags = 0;
        attchmentDescriptions[1].format = specification.depthFormat;
        attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;
        subpassDescription.pDepthStencilAttachment = &depthReference;

        // Use subpass dependencies for layout transitions
        VkSubpassDependency dependencies[2];

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Create the actual renderpass
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 2;
        renderPassInfo.pAttachments = attchmentDescriptions;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescription;
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = dependencies;

        MV_VULKAN(vkCreateRenderPass(graphics.logicalDevice, &renderPassInfo, nullptr, &pass.renderPass));

        //mvRegisterAsset(&am, specification.name, pass.renderPass);

        pass.colorTextures.resize(graphics.swapChainImageViews.size());
        pass.depthTextures.resize(graphics.swapChainImageViews.size());
        pass.frameBuffers.resize(graphics.swapChainImageViews.size());
        for (size_t i = 0; i < graphics.swapChainImageViews.size(); i++)
        {
            pass.colorTextures[i] = create_texture(graphics,
                specification.width, specification.height, specification.colorFormat,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);

            pass.depthTextures[i] = create_texture(graphics,
                specification.width, specification.height, specification.depthFormat,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT);

            VkImageView imageViews[] = { pass.colorTextures[i].imageInfo.imageView, pass.depthTextures[i].imageInfo.imageView };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = pass.renderPass;
            framebufferInfo.attachmentCount = 2;
            framebufferInfo.pAttachments = imageViews;
            framebufferInfo.width = specification.width;
            framebufferInfo.height = specification.height;
            framebufferInfo.layers = 1;
            MV_VULKAN(vkCreateFramebuffer(graphics.logicalDevice, &framebufferInfo, nullptr, &pass.frameBuffers[i]));
        }

        pass.pipelineSpec.width = pass.specification.width;
        pass.pipelineSpec.height = pass.specification.height;
        pass.pipelineSpec.renderPass = pass.renderPass;

        pass.viewport.x = 0.0f;
        pass.viewport.y = pass.specification.height;
        pass.viewport.width = pass.specification.width;
        pass.viewport.height = -pass.specification.height;
        pass.viewport.minDepth = 0.0f;
        pass.viewport.maxDepth = 1.0f;
        pass.extent.width = (u32)pass.specification.width;
        pass.extent.height = (u32)pass.viewport.y;

        return pass;
    }

    void 
    cleanup_pass(mvGraphics& graphics, mvPass& pass)
    {
        vkDeviceWaitIdle(graphics.logicalDevice);
        for (u32 i = 0; i < graphics.swapChainFramebuffers.size(); i++)
        {
            if (pass.specification.hasColor)
            {
                vkDestroySampler(graphics.logicalDevice, pass.colorTextures[i].imageInfo.sampler, nullptr);
                vkDestroyImage(graphics.logicalDevice, pass.colorTextures[i].textureImage, nullptr);
                vkDestroyImageView(graphics.logicalDevice, pass.colorTextures[i].imageInfo.imageView, nullptr);
                vkFreeMemory(graphics.logicalDevice, pass.colorTextures[i].textureImageMemory, nullptr);
            }

            if (pass.specification.hasDepth)
            {
                vkDestroySampler(graphics.logicalDevice, pass.depthTextures[i].imageInfo.sampler, nullptr);
                vkDestroyImage(graphics.logicalDevice, pass.depthTextures[i].textureImage, nullptr);
                vkDestroyImageView(graphics.logicalDevice, pass.depthTextures[i].imageInfo.imageView, nullptr);
                vkFreeMemory(graphics.logicalDevice, pass.depthTextures[i].textureImageMemory, nullptr);
            }

            vkDestroyFramebuffer(graphics.logicalDevice, pass.frameBuffers[i], nullptr);

        }

        vkDestroyRenderPass(graphics.logicalDevice, pass.renderPass, nullptr);

        pass.renderPass = VK_NULL_HANDLE;
    }

    mvPass
    create_offscreen_pass(mvGraphics& graphics, mvAssetManager& am, mvPassSpecification specification)
    {

        mvPass pass{};
        pass.specification = specification;

        VkAttachmentDescription attchmentDescriptions[2];

        // Color attachment
        attchmentDescriptions[0].flags = 0;
        attchmentDescriptions[0].format = specification.colorFormat;
        attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        
        // Depth attachment
        attchmentDescriptions[1].flags = 0;
        attchmentDescriptions[1].format = specification.depthFormat;
        attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;
        subpassDescription.pDepthStencilAttachment = &depthReference;

        // Use subpass dependencies for layout transitions
        VkSubpassDependency dependencies[2];

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Create the actual renderpass
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 2;
        renderPassInfo.pAttachments = attchmentDescriptions;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescription;
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = dependencies;

        MV_VULKAN(vkCreateRenderPass(graphics.logicalDevice, &renderPassInfo, nullptr, &pass.renderPass));

        pass.colorTextures.resize(graphics.swapChainImageViews.size());
        pass.depthTextures.resize(graphics.swapChainImageViews.size());
        pass.frameBuffers.resize(graphics.swapChainImageViews.size());
        for (size_t i = 0; i < graphics.swapChainImageViews.size(); i++)
        {
            pass.colorTextures[i] = create_texture(graphics,
                specification.width, specification.height, specification.colorFormat,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);

            pass.depthTextures[i] = create_texture(graphics,
                specification.width, specification.height, specification.depthFormat,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT);

            VkImageView imageViews[] = { pass.colorTextures[i].imageInfo.imageView, pass.depthTextures[i].imageInfo.imageView };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = pass.renderPass;
            framebufferInfo.attachmentCount = 2;
            framebufferInfo.pAttachments = imageViews;
            framebufferInfo.width = specification.width;
            framebufferInfo.height = specification.height;
            framebufferInfo.layers = 1;
            MV_VULKAN(vkCreateFramebuffer(graphics.logicalDevice, &framebufferInfo, nullptr, &pass.frameBuffers[i]));
        }

        pass.pipelineSpec.width = pass.specification.width;
        pass.pipelineSpec.height = pass.specification.height;
        pass.pipelineSpec.renderPass = pass.renderPass;

        pass.viewport.x = 0.0f;
        pass.viewport.y = pass.specification.height;
        pass.viewport.width = pass.specification.width;
        pass.viewport.height = -pass.specification.height;
        pass.viewport.minDepth = 0.0f;
        pass.viewport.maxDepth = 1.0f;
        pass.extent.width = (u32)pass.specification.width;
        pass.extent.height = (u32)pass.viewport.y;

        return pass;
    }

    mvPass
    create_depth_pass(mvGraphics& graphics, mvAssetManager& am, mvPassSpecification specification)
    {
        mvPass pass{};
        pass.specification = specification;

        // Depth attachment
        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.flags = 0;
        attachmentDescription.format = specification.depthFormat;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// Clear depth at beginning of the render pass
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// We will read from depth, so it's important to store the depth attachment results
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// We don't care about initial layout of the attachment
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;// Attachment will be transitioned to shader read at render pass end

        VkAttachmentReference depthReference = {};
        depthReference.attachment = 0;
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;			// Attachment will be used as depth/stencil during render pass

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 0;													// No color attachments
        subpass.pDepthStencilAttachment = &depthReference;									// Reference to our depth attachment

        // Use subpass dependencies for layout transitions
        VkSubpassDependency dependencies[2];

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Create the actual renderpass
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &attachmentDescription;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = dependencies;

        MV_VULKAN(vkCreateRenderPass(graphics.logicalDevice, &renderPassInfo, nullptr, &pass.renderPass));

        pass.depthTextures.resize(graphics.swapChainImageViews.size());
        pass.frameBuffers.resize(graphics.swapChainImageViews.size());
        for (size_t i = 0; i < graphics.swapChainImageViews.size(); i++)
        {

            pass.depthTextures[i] = create_texture(graphics,
                specification.width, specification.height, specification.depthFormat,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT);

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = pass.renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &pass.depthTextures[i].imageInfo.imageView;
            framebufferInfo.width = specification.width;
            framebufferInfo.height = specification.height;
            framebufferInfo.layers = 1;
            MV_VULKAN(vkCreateFramebuffer(graphics.logicalDevice, &framebufferInfo, nullptr, &pass.frameBuffers[i]));
        }

        pass.pipelineSpec.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pass.pipelineSpec.width = pass.specification.width;
        pass.pipelineSpec.height = pass.specification.height;
        pass.pipelineSpec.renderPass = pass.renderPass;

        pass.viewport.x = 0.0f;
        pass.viewport.y = pass.specification.height;
        pass.viewport.width = pass.specification.width;
        pass.viewport.height = -pass.specification.height;
        pass.viewport.minDepth = 0.0f;
        pass.viewport.maxDepth = 1.0f;
        pass.extent.width = (u32)pass.specification.width;
        pass.extent.height = (u32)pass.viewport.y;

        return pass;
    }
}