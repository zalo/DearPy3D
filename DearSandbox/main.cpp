#include <imgui.h>
#include "mvContext.h"
#include "mvMesh.h"
#include "mvCamera.h"
#include "mvTimer.h"
#include "mvPointLight.h"
#include "mvMaterial.h"
#include "mvContext.h"
#include <iostream>
#include "mvMath.h"
#include "mvRenderer.h"

using namespace DearPy3D;

int main() 
{
    CreateContext();
    
    Renderer::mvStartRenderer();

    mvCamera camera{};
    camera.pos = glm::vec3{5.0f, 5.0f, -15.0f};
    camera.aspect = GContext->viewport.width/GContext->viewport.height;
    
    mvMesh quad1 = mvCreateTexturedQuad("../../Resources/brickwall.jpg");
    quad1.pos.x = 5.0f;
    quad1.pos.y = 5.0f;
    quad1.pos.z = 5.0f;
    mvMesh cube1 = mvCreateTexturedCube("../../Resources/brickwall.jpg");
    cube1.pos.x = 10.0f;
    cube1.pos.y = 10.0f;
    cube1.pos.z = 10.0f;
    mvMesh lightSphere = mvCreateSolidSphere();
    lightSphere.pos.y = 10.0f;

    auto mat1 = mvMaterialData{};
    auto mat2 = mvMaterialData{};
    auto mat3 = mvMaterialData{};
    mat1.materialColor = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
    mat2.materialColor = glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f };
    mat3.materialColor = glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f };

    mvMaterial material = mvCreateMaterial({mat1, mat2, mat3}, "vs_shader.vert.spv", "ps_shader.frag.spv");
    mvPointLight light = mvCreatePointLight(glm::vec3{1.0f, 0.0f, 0.0f});
    mvFinalizePipeline(material.pipeline, {material.descriptorSetLayout, light.descriptorSetLayout});

    //---------------------------------------------------------------------
    // main loop
    //---------------------------------------------------------------------
    mvTimer timer;

    while (GContext->viewport.running)
    {
        const auto dt = timer.mark() * 1.0f;

        mvProcessViewportEvents();

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
            mvCleanupMaterial(material);
            mvCleanupMesh(cube1);
            mvCleanupMesh(lightSphere);
            mvCleanupMesh(quad1);
            GContext->viewport.width = newwidth;
            GContext->viewport.height = newheight;
            Renderer::mvResize();

            // recreation
            auto newquad1 = mvCreateTexturedQuad("../../Resources/brickwall.jpg");
            auto newcube1 = mvCreateTexturedCube("../../Resources/brickwall.jpg");
            auto newlightsphere = mvCreateSolidSphere();
            
            quad1.indexBuffer = newquad1.indexBuffer;
            quad1.vertexBuffer = newquad1.vertexBuffer;
            cube1.indexBuffer = newcube1.indexBuffer;
            cube1.vertexBuffer = newcube1.vertexBuffer;
            lightSphere.indexBuffer = newlightsphere.indexBuffer;
            lightSphere.vertexBuffer = newlightsphere.vertexBuffer;

            material = mvCreateMaterial({ mat1, mat2, mat3 }, "vs_shader.vert.spv", "ps_shader.frag.spv");

            GContext->viewport.resized = false;

            // hacky but works
            // window only seems to repaint if moved, so we move the window
            // slightly
            int x, y;
            glfwGetWindowPos(GContext->viewport.handle, &x, &y);
            glfwSetWindowPos(GContext->viewport.handle, ++x, y);
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
        Renderer::mvBeginFrame();

        //---------------------------------------------------------------------
        // main pass
        //---------------------------------------------------------------------

        auto currentCommandBuffer = mvGetCurrentCommandBuffer();

        Renderer::mvBeginPass(currentCommandBuffer, GContext->graphics.renderPass);

        ImGuiIO& io = ImGui::GetIO();
        ImGui::GetForegroundDrawList()->AddText(ImVec2(45, 45),
            ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(io.Framerate) + " FPS").c_str());

        ImGui::Begin("Settings");
        if(ImGui::Button("Change Material"))
        {
            auto newmat1 = mvMaterialData{};
            auto newmat2 = mvMaterialData{};
            auto newmat3 = mvMaterialData{};
            newmat1.materialColor = glm::vec4{ 1.0f, 1.0f, 0.0f, 1.0f };
            newmat2.materialColor = glm::vec4{ 0.0f, 1.0f, 1.0f, 1.0f };
            newmat3.materialColor = glm::vec4{ 1.0f, 0.0f, 1.0f, 1.0f };

            std::vector<mvMaterialData> newmaterials = {newmat1, newmat2, newmat3};

            mvUpdateBuffer(material.materialBuffer.buffers[GContext->graphics.currentImageIndex], newmaterials.data());
        }
        ImGui::End();

        glm::mat4 viewMatrix = mvBuildCameraMatrix(camera);
        glm::mat4 projMatrix = mvBuildProjectionMatrix(camera);

        mvBind(light, material.pipeline.pipelineLayout);
        Renderer::mvRenderMesh(lightSphere, material, {}, viewMatrix, projMatrix);

        Renderer::mvRenderMesh(cube1, material, {}, viewMatrix, projMatrix);
        Renderer::mvRenderMesh(quad1, material, {}, viewMatrix, projMatrix);
        
        Renderer::mvEndPass(currentCommandBuffer);

        //---------------------------------------------------------------------
        // submit command buffers & present
        //---------------------------------------------------------------------
        Renderer::mvEndFrame();
        Renderer::mvPresent();
    }

    mvCleanupMaterial(material);
    mvCleanupPointLight(light);
    mvCleanupMesh(cube1);
    mvCleanupMesh(lightSphere);
    mvCleanupMesh(quad1);
    Renderer::mvStopRenderer();
    DestroyContext();
}

