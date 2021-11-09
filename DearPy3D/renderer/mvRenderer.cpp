#include "mvRenderer.h"
#include <stdexcept>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include "mvAssetManager.h"

namespace Renderer {

    mv_internal void
    mvSetupImGui(GLFWwindow* window)
    {

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = mvGetVkInstance();
        init_info.PhysicalDevice = mvGetPhysicalDevice();
        init_info.Device = mvGetLogicalDevice();
        init_info.QueueFamily = GContext->graphics.graphicsQueueFamily;
        init_info.Queue = GContext->graphics.graphicsQueue;
        init_info.PipelineCache = nullptr;
        init_info.DescriptorPool = GContext->graphics.descriptorPool;
        init_info.Allocator = nullptr;
        init_info.MinImageCount = GContext->graphics.minImageCount;
        init_info.ImageCount = GContext->graphics.minImageCount;
        init_info.CheckVkResultFn = nullptr;
        ImGui_ImplVulkan_Init(&init_info, GContext->graphics.renderPass);

        // Upload Fonts
        {
            // Use any command queue
            VkCommandBuffer command_buffer = mvBeginSingleTimeCommands();
            ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
            mvEndSingleTimeCommands(command_buffer);
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }

    }

    mv_internal void
    mvCleanupImGui()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    mv_internal void
    mvResizeImGui()
    {
        vkDeviceWaitIdle(mvGetLogicalDevice());
        ImGui_ImplVulkan_SetMinImageCount(GContext->graphics.minImageCount);
    }

    mv_internal void
    mvBeginImGuiFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    mv_internal void
    mvEndImGuiFrame()
    {
        ImGui::Render();

        ImDrawData* main_draw_data = ImGui::GetDrawData();

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData(main_draw_data, mvGetCurrentCommandBuffer());
    }

    void
    mvResize()
    {
        mvRecreateSwapChain();
        mvResizeImGui();
    }

    void
    mvStartRenderer()
    {
        mvInitializeViewport(500, 500);
        GContext->IO.shaderDirectory = "../../DearPy3D/shaders/";
        GContext->graphics.enableValidationLayers = true;
        GContext->graphics.validationLayers = { "VK_LAYER_KHRONOS_validation" };
        GContext->graphics.deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        mvSetupGraphicsContext();
        mvSetupImGui(GContext->viewport.handle);
    }

    void
    mvStopRenderer()
    {
        mvCleanupImGui();
        mvCleanupGraphicsContext();
    }

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
    mvPresent()
    {

        VkSemaphore signalSemaphores[] = { GContext->graphics.renderFinishedSemaphores[GContext->graphics.currentFrame] };

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { GContext->graphics.swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &GContext->graphics.currentImageIndex;

        VkResult result = vkQueuePresentKHR(GContext->graphics.presentQueue, &presentInfo);

        GContext->graphics.currentFrame = (GContext->graphics.currentFrame + 1) % MV_MAX_FRAMES_IN_FLIGHT;
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
            mvUpdateMaterialDescriptors(am, am.phongMaterials[mesh.phongMaterialID].material, mesh.diffuseTexture);
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
    mvBeginPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("failed to begin recording command buffer!");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = GContext->graphics.swapChainFramebuffers[GContext->graphics.currentImageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = GContext->graphics.swapChainExtent;

        VkClearValue clearValues[2];
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        mvBeginImGuiFrame();
    }

    void
    mvEndPass(VkCommandBuffer commandBuffer)
    {
        mvEndImGuiFrame();

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

    void 
    mvPreLoadAssets(mvAssetManager& am)
    {
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        descriptorSetLayouts.resize(2);

        {
            //-----------------------------------------------------------------------------
            // create descriptor set layout
            //-----------------------------------------------------------------------------
            std::vector<VkDescriptorSetLayoutBinding> bindings;
            bindings.resize(2);

            bindings[0].binding = 0u;
            bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindings[0].descriptorCount = 1;
            bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[0].pImmutableSamplers = nullptr;

            bindings[1].binding = 1u;
            bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindings[1].descriptorCount = 1;
            bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[1].pImmutableSamplers = nullptr;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<u32>(bindings.size());
            layoutInfo.pBindings = bindings.data();

            if (vkCreateDescriptorSetLayout(mvGetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayouts[0]) != VK_SUCCESS)
                throw std::runtime_error("failed to create descriptor set layout!");

            mvRegisterDescriptorSetLayoutAsset(&am, descriptorSetLayouts[0], "scene");
        }

        {
            //-----------------------------------------------------------------------------
            // create descriptor set layout
            //-----------------------------------------------------------------------------
            std::vector<VkDescriptorSetLayoutBinding> bindings;
            bindings.resize(2);

            bindings[0].binding = 0u;
            bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[0].descriptorCount = 1;
            bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[0].pImmutableSamplers = nullptr;

            bindings[1].binding = 1u;
            bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindings[1].descriptorCount = 1;
            bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[1].pImmutableSamplers = nullptr;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<u32>(bindings.size());
            layoutInfo.pBindings = bindings.data();

            if (vkCreateDescriptorSetLayout(mvGetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayouts[1]) != VK_SUCCESS)
                throw std::runtime_error("failed to create descriptor set layout!");

            mvRegisterDescriptorSetLayoutAsset(&am, descriptorSetLayouts[1], "phong");
        }

        mvAssetID pipelineLayout = mvGetPipelineLayoutAsset(&am, descriptorSetLayouts);
    }
}