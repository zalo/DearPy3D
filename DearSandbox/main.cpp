#include "mvGraphicsContext.h"
#include "mvTexturedQuad.h"
#include "mvCube.h"
#include "mvCamera.h"
#include "mvTimer.h"
#include "mvPointLight.h"
#include "mvMaterial.h"
#include "mvImGuiManager.h"
#include "mvContext.h"
#include <iostream>
#include <array>
#include "mvMath.h"

using namespace DearPy3D;

// forward declarations
void BeginPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass);
void EndPass(VkCommandBuffer commandBuffer);
void RenderDrawable(mvDrawable& drawable, mvPipeline& pipeline, uint32_t index, mvTransforms transforms, glm::mat4 camera, glm::mat4 projection);

int main() 
{
    CreateContext();
    InitializeViewport(500, 500);
    
    // graphics
    GContext->graphics.enableValidationLayers = true;
    GContext->graphics.validationLayers = { "VK_LAYER_KHRONOS_validation" };
    GContext->graphics.deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    mvSetupGraphicsContext();
    
    auto imgui = mvImGuiManager(GContext->viewport.handle);

    mvCamera camera{};
    camera.pos = glm::vec3{5.0f, 5.0f, -15.0f};
    camera.aspect = GContext->viewport.width/GContext->viewport.height;
    
    mvDrawable quad1 = mvCreateTexturedQuad("../../Resources/brickwall.jpg");
    mvDrawable cube1 = mvCreateTexturedCube("../../Resources/brickwall.jpg");
    cube1.pos.x = 10.0f;
    cube1.pos.y = 10.0f;
    cube1.pos.z = 10.0f;

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

            camera.aspect = (float)newwidth/(float)newheight;

            // cleanup
            material->cleanup();
            mvCleanupDrawable(cube1);
            mvCleanupDrawable(quad1);
            GContext->viewport.width = newwidth;
            GContext->viewport.height = newheight;
            mvRecreateSwapChain();
            imgui.resize();

            // recreation
            auto newquad1 = mvCreateTexturedQuad("../../Resources/brickwall.jpg");
            auto newcube1 = mvCreateTexturedCube("../../Resources/brickwall.jpg");

            quad1.indexBuffer = newquad1.indexBuffer;
            quad1.vertexBuffer = newquad1.vertexBuffer;
            cube1.indexBuffer = newcube1.indexBuffer;
            cube1.vertexBuffer = newcube1.vertexBuffer;

            material = std::make_shared<mvMaterial>();

            GContext->viewport.resized = false;

        }

        //---------------------------------------------------------------------
        // input handling
        //---------------------------------------------------------------------
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_W) == GLFW_PRESS) mvTranslateCamera(camera, 0.0f, 0.0f, dt);
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_S) == GLFW_PRESS) mvTranslateCamera(camera, 0.0f, 0.0f, -dt);
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_A) == GLFW_PRESS) mvTranslateCamera(camera, -dt, 0.0f, 0.0f);
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_D) == GLFW_PRESS) mvTranslateCamera(camera, dt, 0.0f, 0.0f);
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_R) == GLFW_PRESS) mvTranslateCamera(camera, 0.0f, dt, 0.0f);
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_F) == GLFW_PRESS) mvTranslateCamera(camera, 0.0f, -dt, 0.0f);
        if (!GContext->viewport.cursorEnabled) mvRotateCamera(camera, GContext->viewport.deltaX, GContext->viewport.deltaY);
        
        //---------------------------------------------------------------------
        // wait for fences and acquire next image
        //---------------------------------------------------------------------
        mvBeginFrame();

        //---------------------------------------------------------------------
        // main pass
        //---------------------------------------------------------------------
        
        auto currentCommandBuffer = mvGetCurrentCommandBuffer();

        BeginPass(currentCommandBuffer, GContext->graphics.renderPass);
        imgui.beginFrame();

        glm::mat4 viewMatrix = mvBuildCameraMatrix(camera);
        glm::mat4 projMatrix = mvBuildProjectionMatrix(camera);

        material->bind(0, mat1);
        RenderDrawable(cube1, material->getPipeline(), 0, {}, viewMatrix, projMatrix);

        material->bind(1, mat2);
        RenderDrawable(quad1, material->getPipeline(), 1, {}, viewMatrix, projMatrix);

        imgui.endFrame();
        EndPass(currentCommandBuffer);

        //---------------------------------------------------------------------
        // submit command buffers & present
        //---------------------------------------------------------------------
        mvEndFrame();
        mvPresent();
    }

    material->cleanup();
    mvCleanupDrawable(cube1);
    mvCleanupDrawable(quad1);
    imgui.cleanup();
    mvCleanupGraphicsContext();
    DestroyContext();
}

void RenderDrawable(mvDrawable& drawable, mvPipeline& pipeline, uint32_t index, mvTransforms transforms, glm::mat4 camera, glm::mat4 projection)
{
    drawable.indexBuffer->bind();
    drawable.vertexBuffer->bind();
    pipeline.bind(index);

    glm::mat4 localTransform = glm::translate(glm::vec3{drawable.pos.x, drawable.pos.y, drawable.pos.z}) *
        glm::rotate(drawable.rot.x, glm::vec3{1.0f, 0.0f, 0.0f }) *
        glm::rotate(drawable.rot.y, glm::vec3{0.0f, 1.0f, 0.0f }) *
        glm::rotate(drawable.rot.z, glm::vec3{0.0f, 0.0f, 1.0f });

    transforms.model = localTransform;
    transforms.modelView = camera * transforms.model;
    transforms.modelViewProjection = projection * transforms.modelView;

    vkCmdPushConstants(
        GContext->graphics.commandBuffers[GContext->graphics.currentImageIndex],
        pipeline.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvTransforms), &transforms);

    mvDraw(drawable.indexBuffer->getVertexCount());
}

void BeginPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass)
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

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void EndPass(VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
}
