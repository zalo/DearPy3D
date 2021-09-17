#include "mvGraphicsContext.h"
#include "mvTexturedQuad.h"
#include "mvCube.h"
#include "mvCamera.h"
#include "mvTimer.h"
#include "mvRenderer.h"
#include "mvPointLight.h"
#include "mvMaterial.h"
#include "mvImGuiManager.h"
#include "mvContext.h"

using namespace DearPy3D;

int main() 
{
    CreateContext();
    InitializeViewport(500, 500);
    
    // graphics
    GContext->graphics.instance = mvCreateVulkanInstance({ "VK_LAYER_KHRONOS_validation" });
    GContext->graphics.surface = mvCreateSurface(GContext->viewport.handle);
    GContext->graphics.physicalDevice = mvCreatePhysicalDevice(GContext->graphics.deviceProperties, { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
    GContext->graphics.logicalDevice = mvCreateLogicalDevice(GContext->graphics.physicalDevice);
    mvAllocator::Init();
    CreateSwapChain(GContext->graphics, 500, 500);
    CreateMainCommandPool(GContext->graphics);
    CreateMainDescriptorPool(&(GContext->graphics.descriptorPool));
    CreateMainRenderPass(&(GContext->graphics.renderPass), GContext->graphics.swapChainImageFormat);
    CreateMainDepthResources(&GContext->graphics.depthImage, &GContext->graphics.depthImageView, &GContext->graphics.depthImageMemory);
    CreateFrameBuffers(GContext->graphics.renderPass, GContext->graphics.swapChainFramebuffers, GContext->graphics.swapChainImageViews, GContext->graphics.depthImageView);
    CreateSyncObjects(GContext->graphics.imageAvailableSemaphores, GContext->graphics.renderFinishedSemaphores, GContext->graphics.inFlightFences, GContext->graphics.imagesInFlight);

    auto renderer = mvRenderer();
    auto imgui = mvImGuiManager(GContext->viewport.handle);
    auto camera = mvCamera(GContext->viewport.width, GContext->viewport.height, glm::vec3{5.0f, 5.0f, -15.0f});

    auto quad1 = std::make_shared<mvTexturedQuad>("../../Resources/brickwall.jpg");
    auto cube1 = std::make_shared<mvCube>("../../Resources/brickwall.jpg");
    cube1->setPosition(10, 10, 10);

    auto material = std::make_shared<mvMaterial>();

    auto mat1 = mvMaterialBuffer::mvMaterialData{};
    auto mat2 = mvMaterialBuffer::mvMaterialData{};
    mat1.materialColor = glm::vec4{ 1.0f, 1.0f, 0.0f, 1.0f };
    mat2.materialColor = glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f };

    //---------------------------------------------------------------------
    // main loop
    //---------------------------------------------------------------------
    mvTimer timer;

    while (GContext->viewport.running)
    {
        const auto dt = timer.mark() * 1.0f;

        ProcessViewportEvents();

        //---------------------------------------------------------------------
        // handle window resizing
        //---------------------------------------------------------------------
        if (GContext->viewport.resized)
        {

            int newwidth = 0, newheight = 0;
            glfwGetFramebufferSize(GContext->viewport.handle, &newwidth, &newheight);
            while (newwidth == 0 || newheight == 0)
            {
                glfwGetFramebufferSize(GContext->viewport.handle, &newwidth, &newheight);
                glfwWaitEvents();
            }

            camera.updateProjection(newwidth, newheight);

            // cleanup
            material->cleanup();
            cube1->cleanup();
            quad1->cleanup();
            RecreateSwapChain(newwidth, newheight);
            imgui.resize();

            // recreation
            quad1 = std::make_shared<mvTexturedQuad>("../../Resources/brickwall.jpg");
            cube1 = std::make_shared<mvCube>("../../Resources/brickwall.jpg");
            cube1->setPosition(10, 10, 10);
            material = std::make_shared<mvMaterial>();
        }

        //---------------------------------------------------------------------
        // input handling
        //---------------------------------------------------------------------
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_W) == GLFW_PRESS) camera.translate(0.0f, 0.0f, dt);
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_S) == GLFW_PRESS) camera.translate(0.0f, 0.0f, -dt);
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_A) == GLFW_PRESS) camera.translate(-dt, 0.0f, 0.0f);
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_D) == GLFW_PRESS) camera.translate(dt, 0.0f, 0.0f);
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_R) == GLFW_PRESS) camera.translate(0.0f, dt, 0.0f);
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_F) == GLFW_PRESS) camera.translate(0.0f, -dt, 0.0f);
        if (!GContext->viewport.cursorEnabled) camera.rotate(GContext->viewport.deltaX, GContext->viewport.deltaY);
        
        //---------------------------------------------------------------------
        // wait for fences and acquire next image
        //---------------------------------------------------------------------
        renderer.beginFrame();

        //---------------------------------------------------------------------
        // main pass
        //---------------------------------------------------------------------
        
        auto currentCommandBuffer = GetCurrentCommandBuffer();

        renderer.beginPass(currentCommandBuffer, GContext->graphics.renderPass);
        imgui.beginFrame();

        ImGui::Begin("Cool window");
        ImGui::Button("Press me");
        ImGui::End();

        renderer.setCamera(camera);

        material->bind(0, mat1);
        renderer.renderDrawable(*cube1, material->getPipeline(), 0);

        material->bind(1, mat2);
        renderer.renderDrawable(*quad1, material->getPipeline(), 1);

        imgui.endFrame();
        renderer.endPass(currentCommandBuffer);

        //---------------------------------------------------------------------
        // submit command buffers & present
        //---------------------------------------------------------------------
        renderer.endFrame();
    }

    material->cleanup();
    cube1->cleanup();
    quad1->cleanup();
    imgui.cleanup();
    CleanupGraphicsContext();
    DestroyContext();
}