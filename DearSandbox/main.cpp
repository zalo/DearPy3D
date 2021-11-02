#include <imgui.h>
#include "mvContext.h"
#include "mvMesh.h"
#include "mvCamera.h"
#include "mvTimer.h"
#include "mvLights.h"
#include "mvMaterials.h"
#include "mvContext.h"
#include <iostream>
#include "mvMath.h"
#include "mvRenderer.h"
#include "mvObjLoader.h"
#include "mvAssetManager.h"

int main() 
{

    CreateContext();
    
    Renderer::mvStartRenderer();

    mvAssetManager am{};
    mvInitializeAssetManager(&am);

    mvCamera camera{};
    camera.pos = {5.0f, 5.0f, -15.0f};
    camera.aspect = GContext->viewport.width/GContext->viewport.height;
    
    mvMesh quad1 = mvCreateTexturedQuad(am);
    quad1.pos.x = 5.0f;
    quad1.pos.y = 5.0f;
    quad1.pos.z = 5.0f;
    mvMesh cube1 = mvCreateTexturedCube(am, 3.0f);
    cube1.pos.x = 10.0f;
    cube1.pos.y = 10.0f;
    cube1.pos.z = 20.0f;
    mvMesh lightCube = mvCreateTexturedCube(am, 0.25f);
    lightCube.pos.y = 10.0f;

    auto mat1 = mvMaterialData{};
    auto mat2 = mvMaterialData{};
    auto mat3 = mvMaterialData{};
    mat1.materialColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    mat1.doLighting = false;
    mat2.materialColor = { 0.0f, 1.0f, 0.0f, 1.0f };
    mat3.materialColor = { 1.0f, 0.0f, 0.0f, 1.0f };

    mvAssetID material1 = mvGetPhongMaterialAsset(&am, mat1, "vs_shader.vert.spv", "ps_shader.frag.spv");
    mvAssetID material2 = mvGetPhongMaterialAsset(&am, mat2, "vs_shader.vert.spv", "ps_shader.frag.spv");
    mvAssetID material3 = mvGetPhongMaterialAsset(&am, mat3, "vs_shader.vert.spv", "ps_shader.frag.spv");
    mvPointLight light = mvCreatePointLight(am, {0.0f, 10.0f, 0.0f});

    mvFinalizePipeline(am.phongMaterials[material1].material.pipeline, { am.phongMaterials[material1].material.descriptorSetLayout, light.descriptorSetLayout });
    mvFinalizePipeline(am.phongMaterials[material2].material.pipeline, { am.phongMaterials[material2].material.descriptorSetLayout, light.descriptorSetLayout });
    mvFinalizePipeline(am.phongMaterials[material3].material.pipeline, { am.phongMaterials[material3].material.descriptorSetLayout, light.descriptorSetLayout});

    //---------------------------------------------------------------------
    // main loop
    //---------------------------------------------------------------------
    mvTimer timer;

    while (GContext->viewport.running)
    {
        const auto dt = timer.mark() * 1.0f;
        //quad1.rot.z += dt;

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
            mvCleanupPointLight(light);
            mvPrepareResizeAssetManager(&am);
            GContext->viewport.width = newwidth;
            GContext->viewport.height = newheight;
            Renderer::mvResize();

            // recreation
            auto newquad1 = mvCreateTexturedQuad(am);
            auto newcube1 = mvCreateTexturedCube(am, 3.0f);
            auto newlightcube = mvCreateTexturedCube(am, 0.25f);
            
            quad1.indexBuffer = newquad1.indexBuffer;
            quad1.vertexBuffer = newquad1.vertexBuffer;
            cube1.indexBuffer = newcube1.indexBuffer;
            cube1.vertexBuffer = newcube1.vertexBuffer;
            lightCube.indexBuffer = newlightcube.indexBuffer;
            lightCube.vertexBuffer = newlightcube.vertexBuffer;

            light = mvCreatePointLight(am, { 0.0f, 10.0f, 0.0f });
            material1 = mvGetPhongMaterialAsset(&am, mat1, "vs_shader.vert.spv", "ps_shader.frag.spv");
            material2 = mvGetPhongMaterialAsset(&am, mat2, "vs_shader.vert.spv", "ps_shader.frag.spv");
            material3 = mvGetPhongMaterialAsset(&am, mat3, "vs_shader.vert.spv", "ps_shader.frag.spv");

            mvFinalizePipeline(am.phongMaterials[material1].material.pipeline, { am.phongMaterials[material1].material.descriptorSetLayout, light.descriptorSetLayout });
            mvFinalizePipeline(am.phongMaterials[material2].material.pipeline, { am.phongMaterials[material2].material.descriptorSetLayout, light.descriptorSetLayout });
            mvFinalizePipeline(am.phongMaterials[material3].material.pipeline, { am.phongMaterials[material3].material.descriptorSetLayout, light.descriptorSetLayout });

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

        ImGui::Begin("Light Controls");
        if (ImGui::SliderFloat3("Position", &light.info.viewLightPos.x, -25.0f, 25.0f))
        {
            lightCube.pos.x = light.info.viewLightPos.x;
            lightCube.pos.y = light.info.viewLightPos.y;
            lightCube.pos.z = light.info.viewLightPos.z;
        }
        ImGui::End();

        mvMat4 viewMatrix = mvBuildCameraMatrix(camera);
        mvMat4 projMatrix = mvBuildProjectionMatrix(camera);

        mvBind(am, light, viewMatrix, am.phongMaterials[material1].material.pipeline.pipelineLayout);
        Renderer::mvRenderMesh(am, lightCube, material1, mvTranslate(mvIdentityMat4(), mvVec3{ 0.0f, 0.0f, 0.0f }), viewMatrix, projMatrix);

        mvMat4 extratransform = mvTranslate(mvIdentityMat4(), mvVec3{ 0.0f, 0.0f, 0.0f });
        Renderer::mvRenderMesh(am, cube1, material2, extratransform, viewMatrix, projMatrix);
        Renderer::mvRenderMesh(am, quad1, material3, extratransform, viewMatrix, projMatrix);

        Renderer::mvEndPass(currentCommandBuffer);

        //---------------------------------------------------------------------
        // submit command buffers & present
        //---------------------------------------------------------------------
        Renderer::mvEndFrame();
        Renderer::mvPresent();
    }

    mvCleanupPointLight(light);

    mvCleanupAssetManager(&am);
    Renderer::mvStopRenderer();
    DestroyContext();
}

