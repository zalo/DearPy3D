#include "mvImGuiManager.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include "mvContext.h"

namespace DearPy3D {

	mvImGuiManager::mvImGuiManager(GLFWwindow* window)
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

	void mvImGuiManager::cleanup()
	{
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
	}

    void mvImGuiManager::resize()
    {
        vkDeviceWaitIdle(mvGetLogicalDevice());
        ImGui_ImplVulkan_SetMinImageCount(GContext->graphics.minImageCount);
    }

	void mvImGuiManager::beginFrame() const
	{
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
	}

	void mvImGuiManager::endFrame() const
	{
        ImGui::Render();

        ImDrawData* main_draw_data = ImGui::GetDrawData();

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData(main_draw_data, GContext->graphics.commandBuffers[GContext->graphics.currentImageIndex]);
	}

}