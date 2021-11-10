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
#include "mvScene.h"

mv_internal const char* gltfModel = "FlightHelmet";
mv_internal const char* sponzaPath = "C:/dev/MarvelAssets/Sponza/";
mv_internal const char* gltfPath = "C://dev//glTF-Sample-Models//2.0//";
mv_internal b8 loadGLTF = false;
mv_internal b8 loadSponza = true;
mv_internal f32 shadowWidth = 100.0f;

int main() 
{

    CreateContext();
    
    Renderer::mvStartRenderer();

    mvAssetManager am{};
    mvInitializeAssetManager(&am);
    Renderer::mvPreLoadAssets(am);

    if (loadSponza) mvLoadOBJAssets(am, sponzaPath, "sponza");

    mvAssetID scene = mvGetSceneAsset(&am, {});

    mvCamera camera{};
    camera.pos = { -13.5f, 6.0f, 3.5f };
    camera.front = { 0.0f, 0.0f, 1.0f };
    camera.up = { 0.0f, -1.0f, 0.0f };
    camera.pitch = 0.0f;
    camera.yaw = 0.0f;
    camera.aspect = GContext->viewport.width / GContext->viewport.height;
    
    mvMesh quad1 = mvCreateTexturedQuad(am);
    mvMat4 quadTransform = mvTranslate(mvIdentityMat4(), mvVec3{ 5.0f, 5.0f, 5.0f });

    mvMesh cube1 = mvCreateTexturedCube(am, 3.0f);
    mvMat4 cubeTransform = mvTranslate(mvIdentityMat4(), mvVec3{ 10.0f, 10.0f, 20.0f });

    mvPointLight light = mvCreatePointLight(am, { 0.0f, 10.0f, 0.0f });
    mvMat4 lightTransform = mvTranslate(mvIdentityMat4(), mvVec3{ 0.0f, 10.0f, 0.0f });

    mvDirectionLight dlight = mvCreateDirectionLight(am, { 0.0, -1.0f, 0.0f });
    
    mvUpdateSceneDescriptors(am, am.scenes[scene].scene, light, dlight);

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
            GContext->viewport.width = newwidth;
            GContext->viewport.height = newheight;
            Renderer::mvResize();
            mvPrepareResizeAssetManager(&am);

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
        mvUpdateCameraFPSCamera(camera, dt, 12.0f, 0.004f);
 
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

        mvShowAssetManager(am);

        ImGui::Begin("Light Controls");
        if (ImGui::SliderFloat3("Position", &light.info.viewLightPos.x, -25.0f, 25.0f))
        {
            lightTransform = mvTranslate(mvIdentityMat4(), light.info.viewLightPos.xyz());
            mvUpdateSceneDescriptors(am, am.scenes[scene].scene, light, dlight);
        }
        ImGui::End();

        ImGui::Begin("Scene");
        ImGui::Checkbox("Diffuse Mapping", (bool*)&am.scenes[scene].scene.data.doDiffuse);
        ImGui::Checkbox("Normal Mapping", (bool*)&am.scenes[scene].scene.data.doNormal);
        ImGui::Checkbox("Specular Mapping", (bool*)&am.scenes[scene].scene.data.doSpecular);
        ImGui::End();

        mvMat4 viewMatrix = mvCreateFPSView(camera);
        mvMat4 projMatrix = mvCreateLookAtProjection(camera);

        mvBindScene(am, scene);
        mvBind(am, light, viewMatrix);
        mvBind(am, dlight, viewMatrix);

        Renderer::mvRenderMesh(am, *light.mesh, lightTransform, viewMatrix, projMatrix);
        Renderer::mvRenderMesh(am, cube1, cubeTransform, viewMatrix, projMatrix);
        Renderer::mvRenderMesh(am, quad1, quadTransform, viewMatrix, projMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::mvRenderScene(am, am.scenes[i].scene, viewMatrix, projMatrix);

        Renderer::mvEndPass(currentCommandBuffer);

        //---------------------------------------------------------------------
        // submit command buffers & present
        //---------------------------------------------------------------------
        Renderer::mvEndFrame();
        Renderer::mvPresent();
    }

    mvCleanupAssetManager(&am);
    Renderer::mvStopRenderer();
    DestroyContext();
}