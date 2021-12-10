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
mv_internal b8 loadSponza = true;
mv_internal ImVec2 oldContentRegion = ImVec2(500, 500);

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

    mvAssetID scene = mvRegisterAsset(&am, "test_scene", mvCreateScene(am, {}));

    if (loadSponza) mvLoadOBJAssets(am, sponzaPath, "sponza");

    mvSkybox skybox = mvCreateSkybox(am);

    mvSceneData sceneData{};

    mvCamera camera{};
    camera.aspect = oldContentRegion.x / oldContentRegion.y;

    f32 angle = 10.0f;
    f32 zcomponent = sinf(MV_PI * angle / 180.0f);
    f32 ycomponent = cosf(MV_PI * angle / 180.0f);

    f32 offscreenWidth = 75.0f;
    mvOrthoCamera secondaryCamera{};
    secondaryCamera.pos = { 0.0f, 100.0f, 0.0f };
    secondaryCamera.dir = { 0.0f, -ycomponent, zcomponent };
    secondaryCamera.up = { 1.0f, 0.0f, 0.0f };
    secondaryCamera.left = -offscreenWidth;
    secondaryCamera.right = offscreenWidth;
    secondaryCamera.bottom = -offscreenWidth;
    secondaryCamera.top = offscreenWidth;
    secondaryCamera.nearZ = -121.0f;
    secondaryCamera.farZ = 121.0f;
    
    mvPointLight light1 = mvCreatePointLight(am, "light1", { 0.0f, 10.0f, 0.0f });
    mvMat4 lightTransform = mvTranslate(mvIdentityMat4(), mvVec3{ 0.0f, 10.0f, 0.0f });
    mvDirectionLight dlight1 = mvCreateDirectionLight(am, "dlight1", mvVec3{ 0.0, -ycomponent, zcomponent });

    mvCamera lightcamera{};
    lightcamera.pos = light1.info.viewLightPos.xyz();
    lightcamera.fieldOfView = M_PI_2;
   
    // passes
    mvPass mainPass = create_main_pass(am);
    mvPass primaryPass = create_primary_pass(am, oldContentRegion.x, oldContentRegion.y);
    mvPass offscreenPass = create_offscreen_pass(am);
    mvPass shadowPass = create_shadow_pass(am);
    mvPass omniShadowPass = create_omnishadow_pass(am);

    mvTexture shadowMapCube = mvCreateCubeTexture(
        1024, 1024,
        VK_FORMAT_R32_SFLOAT,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
        VK_IMAGE_ASPECT_COLOR_BIT);

    mvAssetID cube = mvRegisterAsset(&am, "cubeshadowmap", shadowMapCube);


    //---------------------------------------------------------------------
    // main loop
    //---------------------------------------------------------------------
    mvTimer timer;
    bool recreatePrimaryRender = false;

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

            // cleanup
            GContext->viewport.width = newwidth;
            GContext->viewport.height = newheight;
            mvRecreateSwapChain();
            mainPass = create_main_pass(am);
            GContext->viewport.resized = false;
        }

        if (recreatePrimaryRender)
        {
            Renderer::mvCleanupPass(primaryPass);
            primaryPass = create_primary_pass(am, primaryPass.viewport.width, abs(primaryPass.viewport.height));
            recreatePrimaryRender = false;
        }

        //---------------------------------------------------------------------
        // input handling
        //---------------------------------------------------------------------
        mvUpdateCameraFPSCamera(camera, dt, 12.0f, 1.0f);

        mvMat4 viewMatrix = mvCreateFPSView(camera);
        mvMat4 projMatrix = mvCreateLookAtProjection(camera);
        
        mvMat4 secondaryViewMatrix = mvCreateOrthoView(secondaryCamera);
        mvMat4 secondaryProjMatrix = mvCreateOrthoProjection(secondaryCamera);
        sceneData.pointShadowView = mvCreateLookAtView(lightcamera);
        sceneData.directionalShadowView = secondaryViewMatrix;
        sceneData.directionalShadowProjection = secondaryProjMatrix;
 
        //---------------------------------------------------------------------
        // wait for fences and acquire next image
        //---------------------------------------------------------------------
        Renderer::mvBeginFrame();
        mvUpdateSkyboxDescriptors(am, skybox, mvGetTextureAssetID2(&am, "../../Resources/SkyBox"));
        mvUpdateSceneDescriptors(am, am.scenes[scene].asset,
            mvGetTextureAssetID(&am, shadowPass.specification.name + std::to_string(GContext->graphics.currentImageIndex)),
            cube
        );
        Renderer::mvUpdateDescriptors(am);
        
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
        // omni shadow pass
        //---------------------------------------------------------------------
        for (u32 i = 0; i < 6; i++)
        {

            mvMat4 camera_matrix = mvIdentityMat4();
            mvVec3 look_target{};
            switch (i)
            {
            case 0: // POSITIVE_X
                look_target = light1.info.viewLightPos.xyz() + mvVec3{ 0.0f, 0.0f, 1.0f };
                camera_matrix = mvLookAtRH(light1.info.viewLightPos.xyz(), look_target, mvVec3{ 0.0f, 1.0f, 0.0f });
                break;
            case 1:	// NEGATIVE_X
                look_target = light1.info.viewLightPos.xyz() + mvVec3{ 0.0f, 0.0f, -1.0f };
                camera_matrix = mvLookAtRH(light1.info.viewLightPos.xyz(), look_target, mvVec3{ 0.0f, 1.0f, 0.0f });
                break;
            case 2:	// POSITIVE_Y
                look_target = light1.info.viewLightPos.xyz() + mvVec3{ 0.0f, -1.0f, 0.0f };
                camera_matrix = mvLookAtRH(light1.info.viewLightPos.xyz(), look_target, mvVec3{ 1.0f, 0.0f, 0.0f });
                break;
            case 3:	// NEGATIVE_Y
                look_target = light1.info.viewLightPos.xyz() + mvVec3{ 0.0f, 1.0f, 0.0f };
                camera_matrix = mvLookAtRH(light1.info.viewLightPos.xyz(), look_target, mvVec3{ -1.0f, 0.0f, 0.0f });
                break;
            case 4:	// POSITIVE_Z
                look_target = light1.info.viewLightPos.xyz() + mvVec3{ 1.0f, 0.0f, 0.0f };
                camera_matrix = mvLookAtRH(light1.info.viewLightPos.xyz(), look_target, mvVec3{ 0.0f, 1.0f, 0.0f });
                break;
            case 5:	// NEGATIVE_Z
                look_target = light1.info.viewLightPos.xyz() + mvVec3{ -1.0f, 0.0f, 0.0f };
                camera_matrix = mvLookAtRH(light1.info.viewLightPos.xyz(), look_target, mvVec3{ 0.0f, 1.0f, 0.0f });
                break;
            }

            Renderer::mvBeginPass(am, mvGetCurrentCommandBuffer(), omniShadowPass);

            for (int i = 0; i < am.sceneCount; i++)
                Renderer::mvRenderSceneOmniShadow(am, am.scenes[i].asset, camera_matrix, mvPerspectiveRH(M_PI_2, 1.0f, 0.1f, 400.0f), light1.info.viewLightPos);

            Renderer::mvEndPass(mvGetCurrentCommandBuffer());

            mvTransitionImageLayout(mvGetCurrentCommandBuffer(),
                omniShadowPass.colorTextures[GContext->graphics.currentImageIndex].textureImage,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

            VkImageSubresourceRange cubeFaceSubresourceRange = {};
            cubeFaceSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            cubeFaceSubresourceRange.baseMipLevel = 0;
            cubeFaceSubresourceRange.levelCount = 1;
            cubeFaceSubresourceRange.baseArrayLayer = i;
            cubeFaceSubresourceRange.layerCount = 1;

            mvTransitionImageLayout(mvGetCurrentCommandBuffer(),
                shadowMapCube.textureImage,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                cubeFaceSubresourceRange
            );

            // Copy region for transfer from framebuffer to cube face
            VkImageCopy copyRegion = {};

            copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.srcSubresource.baseArrayLayer = 0;
            copyRegion.srcSubresource.mipLevel = 0;
            copyRegion.srcSubresource.layerCount = 1;
            copyRegion.srcOffset = { 0, 0, 0 };

            copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.dstSubresource.baseArrayLayer = i;
            copyRegion.dstSubresource.mipLevel = 0;
            copyRegion.dstSubresource.layerCount = 1;
            copyRegion.dstOffset = { 0, 0, 0 };

            copyRegion.extent.width = 1024;
            copyRegion.extent.height = 1024;
            copyRegion.extent.depth = 1;

            // Put image copy into command buffer
            vkCmdCopyImage(
                mvGetCurrentCommandBuffer(),
                omniShadowPass.colorTextures[GContext->graphics.currentImageIndex].textureImage,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                shadowMapCube.textureImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &copyRegion);

            mvTransitionImageLayout(mvGetCurrentCommandBuffer(),
                omniShadowPass.colorTextures[GContext->graphics.currentImageIndex].textureImage,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

            mvTransitionImageLayout(mvGetCurrentCommandBuffer(),
                shadowMapCube.textureImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                cubeFaceSubresourceRange);
        }

        //---------------------------------------------------------------------
        // offscreen pass
        //--------------------------------------------------------------------- 
        mvUpdateLightBuffers(am, light1, am.scenes[scene].asset.descriptorSet.descriptors[1].bufferID[GContext->graphics.currentImageIndex], secondaryViewMatrix, 0);
        mvUpdateLightBuffers(am, dlight1, am.scenes[scene].asset.descriptorSet.descriptors[2].bufferID[GContext->graphics.currentImageIndex], secondaryViewMatrix, 0);
        sceneData.camPos = secondaryCamera.pos;
        mvBindScene(am, scene, sceneData, 0);
        Renderer::mvBeginPass(am, mvGetCurrentCommandBuffer(), offscreenPass);

        Renderer::mvRenderMesh(am, *light1.mesh, lightTransform, secondaryViewMatrix, secondaryProjMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::mvRenderScene(am, am.scenes[i].asset, secondaryViewMatrix, secondaryProjMatrix);

        Renderer::mvEndPass(mvGetCurrentCommandBuffer());

        //---------------------------------------------------------------------
        // primary pass
        //---------------------------------------------------------------------
        mvUpdateLightBuffers(am, light1, am.scenes[scene].asset.descriptorSet.descriptors[1].bufferID[GContext->graphics.currentImageIndex], viewMatrix, 1);
        mvUpdateLightBuffers(am, dlight1, am.scenes[scene].asset.descriptorSet.descriptors[2].bufferID[GContext->graphics.currentImageIndex], viewMatrix, 1);
        sceneData.camPos = camera.pos;
        mvBindScene(am, scene, sceneData, 1);
        Renderer::mvBeginPass(am, mvGetCurrentCommandBuffer(), primaryPass);

        Renderer::mvRenderMesh(am, *light1.mesh, lightTransform, viewMatrix, projMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::mvRenderScene(am, am.scenes[i].asset, viewMatrix, projMatrix);

        if (sceneData.doSkybox)
        {
            mvBindSkybox(am, skybox);
            vkCmdBindPipeline(mvGetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mvGetRawPipelineAsset(&am, "skybox_pass")->pipeline);
            Renderer::mvRenderSkybox(am, viewMatrix, projMatrix);
        }

        Renderer::mvEndPass(mvGetCurrentCommandBuffer());

        //---------------------------------------------------------------------
        // main pass
        //---------------------------------------------------------------------
        Renderer::mvBeginPass(am, mvGetCurrentCommandBuffer(), mainPass);

        ImGui::DockSpaceOverViewport(0);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        if (ImGui::Begin("Debug Output", nullptr))
        {
            ImGui::Image(offscreenPass.colorTextures[GContext->graphics.currentImageIndex].imguiID, ImVec2(512, 512));
        }
        ImGui::End();

        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("Model", 0, ImGuiWindowFlags_NoDecoration);

        ImVec2 contentSize = ImGui::GetWindowContentRegionMax();
        camera.aspect = primaryPass.viewport.width / abs(primaryPass.viewport.height);
        ImGui::Image(primaryPass.colorTextures[GContext->graphics.currentImageIndex].imguiID, contentSize);

        if (contentSize.x == oldContentRegion.x && contentSize.y == oldContentRegion.y)
        {
        }
        else
        {
            primaryPass.viewport.width = contentSize.x;
            primaryPass.viewport.height = contentSize.y;
            recreatePrimaryRender = true;
        }

        oldContentRegion = contentSize;

        ImGuiIO& io = ImGui::GetIO();
        ImGui::GetForegroundDrawList()->AddText(ImVec2(ImGui::GetWindowPos().x+45, 45),
            ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(io.Framerate) + " FPS").c_str());

        ImGui::End();



        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        ImGui::Begin("Light Controls");
        ImGui::SliderFloat("omni depthBias", &omniShadowPass.specification.depthBias, 0.0f, 50.0f);
        ImGui::SliderFloat("omni slopeDepthBias", &omniShadowPass.specification.slopeDepthBias, 0.0f, 50.0f);
        ImGui::SliderFloat("directional depthBias", &shadowPass.specification.depthBias, 0.0f, 50.0f);
        ImGui::SliderFloat("directional slopeDepthBias", &shadowPass.specification.slopeDepthBias, 0.0f, 50.0f);
        if (ImGui::SliderFloat3("Position", &light1.info.viewLightPos.x, -50.0f, 50.0f))
        {
            lightTransform = mvTranslate(mvIdentityMat4(), light1.info.viewLightPos.xyz());
            lightcamera.pos = light1.info.viewLightPos.xyz();
        }

        if(ImGui::SliderFloat("Directional Light Angle", &angle, -45.0f, 45.0f))
        {
            zcomponent = sinf(MV_PI * angle / 180.0f);
            ycomponent = cosf(MV_PI * angle / 180.0f);

            secondaryCamera.dir = { 0.0f, -ycomponent, zcomponent };
            dlight1.info.viewLightDir = mvVec3{ 0.0, -ycomponent, zcomponent };
        }
        ImGui::End();

        mvShowSceneControls("Scene", sceneData);

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), mvGetCurrentCommandBuffer());

        Renderer::mvEndPass(mvGetCurrentCommandBuffer());


        //---------------------------------------------------------------------
        // submit command buffers & present
        //---------------------------------------------------------------------
        Renderer::mvEndFrame();
        mvPresent();
    }

    Renderer::mvCleanupPass(primaryPass);
    mvCleanupAssetManager(&am);
    mvCleanupGraphicsContext();
    DestroyContext();
}