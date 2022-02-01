#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_win32.h>
#include "mvGraphics.h"
#include "sTimer.h"
#include "mvRenderer.h"
#include <stdlib.h>
#include "sSemper.h"

int main() 
{
    int result = putenv("VK_LAYER_PATH=..\\..\\Dependencies\\vk_sdk_lite\\Bin");

    Semper::create_context();
    sWindow* viewport = Semper::create_window(500, 500);
    mvGraphics& graphics = *DearPy3D::setup_graphics_context(*viewport, { "VK_LAYER_KHRONOS_validation" });
    mvRendererContext rctx = DearPy3D::create_renderer_context(graphics);

    //---------------------------------------------------------------------
    // load common pipelines
    //---------------------------------------------------------------------

    VkPipelineLayout pipelineLayout;
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;

    MV_VULKAN(vkCreatePipelineLayout(graphics.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout));

    mvAssetID pipelineLayoutID = DearPy3D::register_pipeline_layout(rctx.pipelineManager, "primary_pass", pipelineLayout);

    mvPipelineSpec mainPipelineSpec;
    mainPipelineSpec.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    mainPipelineSpec.backfaceCulling = true;
    mainPipelineSpec.depthTest = true;
    mainPipelineSpec.depthWrite = true;
    mainPipelineSpec.wireFrame = false;
    mainPipelineSpec.blendEnabled = true;
    mainPipelineSpec.vertexShader = "simple.vert.spv";
    mainPipelineSpec.pixelShader = "simple.frag.spv";
    mainPipelineSpec.width = 0.0f;  // viewport
    mainPipelineSpec.height = 0.0f; // viewport
    mainPipelineSpec.renderPass = graphics.renderPass;
    mainPipelineSpec.pipelineLayout = DearPy3D::get_pipeline_layout(rctx.pipelineManager, "primary_pass");
    mainPipelineSpec.layout = DearPy3D::create_vertex_layout({ MV_POS_3D, MV_COLOR_3D });

    mvPassSpecification mainPassSpec;
    mainPassSpec.name = "primary_pass";
    mainPassSpec.pipeline = DearPy3D::reset_pipeline(graphics, rctx.pipelineManager, "primary_pass", DearPy3D::create_pipeline(graphics, mainPipelineSpec));
    mainPassSpec.clearColorValue = { 0.0f, 0.0f, 0.0f, 1.0f };
    mainPassSpec.clearDepthValue = 1.0f;
    mainPassSpec.depthBias = 0.0f;
    mainPassSpec.slopeDepthBias = 0.0f;
    mainPassSpec.width = graphics.swapChainExtent.width;
    mainPassSpec.height = -graphics.swapChainExtent.height;
    mainPassSpec.colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    mainPassSpec.depthFormat = VK_FORMAT_D32_SFLOAT;
    mainPassSpec.hasColor = true;
    mainPassSpec.hasDepth = true;
    mainPassSpec.mainPass = false;

    mvPass mainPass;
    mainPass.pipelineSpec = mainPipelineSpec;
    mainPass.specification = mainPassSpec;
    mainPass.renderPass = graphics.renderPass;
    mainPass.extent = graphics.swapChainExtent;
    mainPass.frameBuffers = graphics.swapChainFramebuffers;
    mainPass.viewport.x = 0.0f;
    mainPass.viewport.y = graphics.swapChainExtent.height;
    mainPass.viewport.width = graphics.swapChainExtent.width;
    mainPass.viewport.height = -(int)graphics.swapChainExtent.height;
    mainPass.viewport.minDepth = 0.0f;
    mainPass.viewport.maxDepth = 1.0f;
    mainPass.extent.width = (u32)graphics.swapChainExtent.width;
    mainPass.extent.height = (u32)mainPass.viewport.y;

    auto vertices = std::vector<float>
    {
        -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
         0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f
    };

    auto indices = std::vector<u32>{ 2u, 1u, 0u };

    mvBufferSpecification vertexBufferSpec{};
    vertexBufferSpec.usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBufferSpec.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    vertexBufferSpec.size = sizeof(f32);
    vertexBufferSpec.components = 6;
    vertexBufferSpec.count = vertices.size()/6;
    vertexBufferSpec.aligned = false;

    mvBufferSpecification indexBufferSpec{};
    indexBufferSpec.usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBufferSpec.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    indexBufferSpec.size = sizeof(u32);
    indexBufferSpec.components = 1u;
    indexBufferSpec.count = indices.size();
    indexBufferSpec.aligned = false;

    mvBuffer vertexBuffer = DearPy3D::create_buffer(graphics, vertexBufferSpec, vertices.data());
    mvBuffer indexBuffer = DearPy3D::create_buffer(graphics, indexBufferSpec, indices.data());

    //---------------------------------------------------------------------
    // main loop
    //---------------------------------------------------------------------
    sTimer timer;
    while (viewport->running)
    {
        const auto dt = timer.mark() * 1.0f;

        Semper::process_window_events(*viewport);
        Semper::new_frame();
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        //---------------------------------------------------------------------
        // handle window resizing
        //---------------------------------------------------------------------
        if (viewport->sizeChanged)
        {
            // cleanup
            DearPy3D::recreate_swapchain(graphics, *viewport);
            mainPipelineSpec.renderPass = graphics.renderPass;
            mainPass.pipelineSpec = mainPipelineSpec;
            mainPass.specification.width = graphics.swapChainExtent.width;
            mainPass.specification.height = -graphics.swapChainExtent.height;
            mainPass.renderPass = graphics.renderPass;
            mainPass.extent = graphics.swapChainExtent;
            mainPass.frameBuffers = graphics.swapChainFramebuffers;
            mainPass.viewport.y = graphics.swapChainExtent.height;
            mainPass.viewport.width = graphics.swapChainExtent.width;
            mainPass.viewport.height = -(int)graphics.swapChainExtent.height;
            mainPass.extent.width = (u32)graphics.swapChainExtent.width;
            mainPass.extent.height = (u32)mainPass.viewport.y;
            mainPass.specification.pipeline = DearPy3D::reset_pipeline(graphics, rctx.pipelineManager, "primary_pass", DearPy3D::create_pipeline(graphics, mainPass.pipelineSpec));
            viewport->sizeChanged = false;
        }

 
        //---------------------------------------------------------------------
        // wait for fences and acquire next image
        //---------------------------------------------------------------------
        DearPy3D::begin_frame(rctx);

        //---------------------------------------------------------------------
        // main pass
        //---------------------------------------------------------------------
        DearPy3D::begin_pass(rctx, DearPy3D::get_current_command_buffer(graphics), mainPass);


        ImGui::ShowDemoWindow();

        mv_local_persist VkDeviceSize offsets = { 0 };
        VkCommandBuffer commandBuffer = graphics.commandBuffers[graphics.currentImageIndex];
        vkCmdBindIndexBuffer(DearPy3D::get_current_command_buffer(graphics), indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindVertexBuffers(DearPy3D::get_current_command_buffer(graphics), 0, 1, &vertexBuffer.buffer, &offsets);
        vkCmdDrawIndexed(DearPy3D::get_current_command_buffer(graphics), indexBuffer.specification.count, 1, 0, 0, 0);

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), DearPy3D::get_current_command_buffer(graphics));

        DearPy3D::end_pass(DearPy3D::get_current_command_buffer(graphics));


        //---------------------------------------------------------------------
        // submit command buffers & present
        //---------------------------------------------------------------------
        DearPy3D::end_frame(rctx);
        DearPy3D::present(graphics);

        Semper::end_frame();
    }

    
    DearPy3D::cleanup_renderer_context(rctx);
    vkDestroyBuffer(graphics.logicalDevice, vertexBuffer.buffer, nullptr);
    vkDestroyBuffer(graphics.logicalDevice, indexBuffer.buffer, nullptr);
    vkFreeMemory(graphics.logicalDevice, indexBuffer.deviceMemory, nullptr);
    vkFreeMemory(graphics.logicalDevice, vertexBuffer.deviceMemory, nullptr);

    // cleanup imgui
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    DearPy3D::cleanup_graphics_context(graphics);
    Semper::cleanup_window(viewport);
}