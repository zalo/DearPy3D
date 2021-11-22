#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <array>
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
#include "mvSkybox.h"
#include <stdlib.h>

#include "passes.h"
#include "pipelines.h"

mv_internal const char* sponzaPath = "C:/dev/MarvelAssets/Sponza/";
mv_internal b8 loadSponza = false;

int main() 
{

    CreateContext();
    int result = putenv("VK_LAYER_PATH=..\\..\\Dependencies\\vk_sdk_lite\\Bin");
    mvInitializeViewport(500, 500);
    GContext->IO.shaderDirectory = "../../DearPy3D/shaders/";
    GContext->graphics.enableValidationLayers = true;
    GContext->graphics.validationLayers = { "VK_LAYER_KHRONOS_validation" };
    GContext->graphics.deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    mvSetupGraphicsContext();

    mvAssetManager am{};
    mvInitializeAssetManager(&am);
    preload_descriptorset_layouts(am);
    preload_pipeline_layouts(am);
    preload_pipelines(am);

    mvAssetID scene = mvRegisterAsset(&am, "test_scene", mvCreateScene(am, {}));

    if (loadSponza) mvLoadOBJAssets(am, sponzaPath, "sponza");

    mvSkybox skybox = mvCreateSkybox(am);

    mvSceneData sceneData{};

    mvCamera camera{};
    camera.aspect = GContext->viewport.width / GContext->viewport.height;

    mvCamera shadowCamera{};
    shadowCamera.aspect = 1.0f;

    f32 offscreenWidth = 75.0f;
    mvOrthoCamera secondaryCamera{};
    secondaryCamera.pos = { 0.0f, 100.0f, 0.0f };
    secondaryCamera.dir = { 0.0f, -1.0, 0.0f };
    secondaryCamera.up = { 1.0f, 0.0f, 0.0f };
    secondaryCamera.left = -offscreenWidth;
    secondaryCamera.right = offscreenWidth;
    secondaryCamera.bottom = -offscreenWidth;
    secondaryCamera.top = offscreenWidth;
    secondaryCamera.nearZ = -101.0f;
    secondaryCamera.farZ = 101.0f;
    
    mvPointLight light1 = mvCreatePointLight(am, "light1", { 0.0f, 10.0f, 0.0f });
    mvMat4 lightTransform = mvTranslate(mvIdentityMat4(), mvVec3{ 0.0f, 10.0f, 0.0f });
    mvDirectionLight dlight1 = mvCreateDirectionLight(am, "dlight1", mvVec3{ 0.0, -1.0f, 0.0f });
   
    // passes
    mvPass offscreenPass = create_offscreen_pass(am);
    mvPass shadowPass = create_shadow_pass(am);
    mvPass mainPass = create_main_pass(am);

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
            GContext->viewport.width = newwidth;
            GContext->viewport.height = newheight;
            mvRecreateSwapChain();
            mvResizeCleanupAssetManager(&am);
            preload_pipelines(am);
            mvResizeUpdateAssetManager(&am);

            mainPass.renderPass = GContext->graphics.renderPass;
            mainPass.frameBuffers = GContext->graphics.swapChainFramebuffers;
            mainPass.viewport.x = 0.0f;
            mainPass.viewport.y = newheight;
            mainPass.viewport.width = newwidth;
            mainPass.viewport.height = -newheight;
            mainPass.viewport.minDepth = 0.0f;
            mainPass.viewport.maxDepth = 1.0f;

            mainPass.extent.width = (u32)newwidth;
            mainPass.extent.height = (u32)mainPass.viewport.y;

            GContext->viewport.resized = false;
        }

        //---------------------------------------------------------------------
        // input handling
        //---------------------------------------------------------------------
        mvUpdateCameraFPSCamera(camera, dt, 12.0f, 1.0f);

        mvMat4 viewMatrix = mvCreateFPSView(camera);
        mvMat4 projMatrix = mvCreateLookAtProjection(camera);

        mvMat4 secondaryViewMatrix = mvCreateOrthoView(secondaryCamera);
        mvMat4 secondaryProjMatrix = mvCreateOrthoProjection(secondaryCamera);
        sceneData.pointShadowView = secondaryViewMatrix;
        sceneData.directionalShadowView = secondaryViewMatrix;
        sceneData.directionalShadowProjection = secondaryProjMatrix;
 
        //---------------------------------------------------------------------
        // wait for fences and acquire next image
        //---------------------------------------------------------------------
        Renderer::mvBeginFrame();
        mvUpdateSkyboxDescriptors(am, skybox, mvGetTextureAssetID2(&am, "../../Resources/SkyBox"));
        mvUpdateSceneDescriptors(am, am.scenes[scene].asset, mvGetTextureAssetID(&am, shadowPass.specification.name + std::to_string(GContext->graphics.currentImageIndex)));
        Renderer::mvUpdateDescriptors(am);
        
        ImGuiIO& io = ImGui::GetIO();
        ImGui::GetForegroundDrawList()->AddText(ImVec2(45, 45),
            ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(io.Framerate) + " FPS").c_str());

        if (ImGui::Begin("Debug Output", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Image(offscreenPass.colorTextures[GContext->graphics.currentImageIndex].imguiID, ImVec2(512, 512));
        }
        ImGui::End();

        ImGui::Begin("Light Controls");
        ImGui::SliderFloat("depthBias", &shadowPass.specification.depthBias, 0.0f, 50.0f);
        ImGui::SliderFloat("slopeDepthBias", &shadowPass.specification.slopeDepthBias, 0.0f, 50.0f);
        if (ImGui::SliderFloat3("Position", &light1.info.viewLightPos.x, -50.0f, 50.0f))
        {
            lightTransform = mvTranslate(mvIdentityMat4(), light1.info.viewLightPos.xyz());
        }
        ImGui::End();

        mvShowSceneControls("Scene", sceneData);
        //mvShowAssetManager(am);

        //---------------------------------------------------------------------
        // shadow pass
        //---------------------------------------------------------------------
        Renderer::mvBeginPass(am, mvGetCurrentCommandBuffer(), shadowPass);

        Renderer::mvRenderMeshShadow(am, *light1.mesh, lightTransform, secondaryViewMatrix, secondaryProjMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::mvRenderSceneShadow(am, am.scenes[i].asset, secondaryViewMatrix, secondaryProjMatrix);

        Renderer::mvEndPass(mvGetCurrentCommandBuffer());

        //---------------------------------------------------------------------
        // offscreen pass
        //---------------------------------------------------------------------
        
        mvUpdateLightBuffers(am, light1, am.scenes[scene].asset.descriptorSet.descriptors[1].bufferID[GContext->graphics.currentImageIndex], secondaryViewMatrix, 0);
        mvUpdateLightBuffers(am, dlight1, am.scenes[scene].asset.descriptorSet.descriptors[2].bufferID[GContext->graphics.currentImageIndex], secondaryViewMatrix, 0);
        mvBindScene(am, scene, sceneData, 0);
        Renderer::mvBeginPass(am, mvGetCurrentCommandBuffer(), offscreenPass);

        Renderer::mvRenderMesh(am, *light1.mesh, lightTransform, secondaryViewMatrix, secondaryProjMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::mvRenderScene(am, am.scenes[i].asset, secondaryViewMatrix, secondaryProjMatrix);

        Renderer::mvEndPass(mvGetCurrentCommandBuffer());

        //---------------------------------------------------------------------
        // main pass
        //---------------------------------------------------------------------
        mvUpdateLightBuffers(am, light1, am.scenes[scene].asset.descriptorSet.descriptors[1].bufferID[GContext->graphics.currentImageIndex], viewMatrix, 1);
        mvUpdateLightBuffers(am, dlight1, am.scenes[scene].asset.descriptorSet.descriptors[2].bufferID[GContext->graphics.currentImageIndex], viewMatrix, 1);
        mvBindScene(am, scene, sceneData, 1);
        Renderer::mvBeginPass(am, mvGetCurrentCommandBuffer(), mainPass);

        Renderer::mvRenderMesh(am, *light1.mesh, lightTransform, viewMatrix, projMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::mvRenderScene(am, am.scenes[i].asset, viewMatrix, projMatrix);

        if (sceneData.doSkybox)
        {
            mvBindSkybox(am, skybox);
            vkCmdBindPipeline(mvGetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mvGetRawPipelineAsset(&am, "skybox_pass")->pipeline);
            Renderer::mvRenderSkybox(am, viewMatrix, projMatrix);
        }

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), mvGetCurrentCommandBuffer());

        Renderer::mvEndPass(mvGetCurrentCommandBuffer());


        //---------------------------------------------------------------------
        // submit command buffers & present
        //---------------------------------------------------------------------
        Renderer::mvEndFrame();
        mvPresent();
    }

    mvCleanupAssetManager(&am);
    mvCleanupGraphicsContext();
    DestroyContext();
}