#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
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
//#include <crtdbg.h>

mv_internal const char* sponzaPath = "C:/dev/MarvelAssets/Sponza/";
mv_internal b8 loadSponza = true;
mv_internal ImVec2 oldContentRegion = ImVec2(500, 500);

int main() 
{
    //_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF);
    CreateContext();
    int result = putenv("VK_LAYER_PATH=..\\..\\Dependencies\\vk_sdk_lite\\Bin");
    mvInitializeViewport(500, 500);
    GContext->IO.shaderDirectory = "../../DearPy3D/shaders/";
    GContext->graphics.enableValidationLayers = true;
    GContext->graphics.validationLayers = { "VK_LAYER_KHRONOS_validation" };
    GContext->graphics.deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    setup_graphics_context(GContext->graphics, GContext->viewport);

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
    camera.pos = { -18.0f, 12.0f, 1.0f };
    camera.pitch = 0.0f;
    camera.yaw = -M_PI_2;

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
    
    mvPointLight light1 = mvCreatePointLight(am, "light1", { 40.0f, 10.0f, 0.0f });
    mvMat4 lightTransform = mvTranslate(mvIdentityMat4(), mvVec3{ 40.0f, 10.0f, 0.0f });
    mvDirectionLight dlight1 = mvCreateDirectionLight(am, "dlight1", mvVec3{ 0.0, -ycomponent, zcomponent });

    mvCamera lightcamera{};
    lightcamera.pos = light1.info.worldPos.xyz();
    lightcamera.fieldOfView = M_PI_2;
   
    // passes
    mvPass mainPass = create_main_pass(am);
    mvPass primaryPass = create_primary_pass(am, oldContentRegion.x, oldContentRegion.y);
    mvPass offscreenPass = create_offscreen_pass(am);
    mvPass shadowPass = create_shadow_pass(am);
    mvPass omniShadowPass = create_omnishadow_pass(am);

    mvTexture shadowMapCube = create_texture_cube(GContext->graphics,
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
            recreate_swapchain(GContext->graphics);
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
        Renderer::mvBeginPass(am, get_current_command_buffer(GContext->graphics), shadowPass);

        Renderer::mvRenderMeshShadow(am, *light1.mesh, lightTransform, secondaryViewMatrix, secondaryProjMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::mvRenderSceneShadow(am, am.scenes[i].asset, secondaryViewMatrix, secondaryProjMatrix);

        Renderer::mvEndPass(get_current_command_buffer(GContext->graphics));

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
                look_target = light1.info.worldPos.xyz() + mvVec3{ 0.0f, 0.0f, 1.0f };
                camera_matrix = mvLookAtRH(light1.info.worldPos.xyz(), look_target, mvVec3{ 0.0f, 1.0f, 0.0f });
                break;
            case 1:	// NEGATIVE_X
                look_target = light1.info.worldPos.xyz() + mvVec3{ 0.0f, 0.0f, -1.0f };
                camera_matrix = mvLookAtRH(light1.info.worldPos.xyz(), look_target, mvVec3{ 0.0f, 1.0f, 0.0f });
                break;
            case 2:	// POSITIVE_Y
                look_target = light1.info.worldPos.xyz() + mvVec3{ 0.0f, -1.0f, 0.0f };
                camera_matrix = mvLookAtRH(light1.info.worldPos.xyz(), look_target, mvVec3{ 1.0f, 0.0f, 0.0f });
                break;
            case 3:	// NEGATIVE_Y
                look_target = light1.info.worldPos.xyz() + mvVec3{ 0.0f, 1.0f, 0.0f };
                camera_matrix = mvLookAtRH(light1.info.worldPos.xyz(), look_target, mvVec3{ -1.0f, 0.0f, 0.0f });
                break;
            case 4:	// POSITIVE_Z
                look_target = light1.info.worldPos.xyz() + mvVec3{ 1.0f, 0.0f, 0.0f };
                camera_matrix = mvLookAtRH(light1.info.worldPos.xyz(), look_target, mvVec3{ 0.0f, 1.0f, 0.0f });
                break;
            case 5:	// NEGATIVE_Z
                look_target = light1.info.worldPos.xyz() + mvVec3{ -1.0f, 0.0f, 0.0f };
                camera_matrix = mvLookAtRH(light1.info.worldPos.xyz(), look_target, mvVec3{ 0.0f, 1.0f, 0.0f });
                break;
            }

            Renderer::mvBeginPass(am, get_current_command_buffer(GContext->graphics), omniShadowPass);

            for (int i = 0; i < am.sceneCount; i++)
                Renderer::mvRenderSceneOmniShadow(am, am.scenes[i].asset, camera_matrix, mvPerspectiveRH(M_PI_2, 1.0f, 0.1f, 400.0f), light1.info.worldPos);

            Renderer::mvEndPass(get_current_command_buffer(GContext->graphics));

            transition_image_layout(get_current_command_buffer(GContext->graphics),
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

            transition_image_layout(get_current_command_buffer(GContext->graphics),
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
                get_current_command_buffer(GContext->graphics),
                omniShadowPass.colorTextures[GContext->graphics.currentImageIndex].textureImage,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                shadowMapCube.textureImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &copyRegion);

            transition_image_layout(get_current_command_buffer(GContext->graphics),
                omniShadowPass.colorTextures[GContext->graphics.currentImageIndex].textureImage,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

            transition_image_layout(get_current_command_buffer(GContext->graphics),
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
        Renderer::mvBeginPass(am, get_current_command_buffer(GContext->graphics), offscreenPass);

        Renderer::mvRenderMesh(am, *light1.mesh, lightTransform, secondaryViewMatrix, secondaryProjMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::mvRenderScene(am, am.scenes[i].asset, secondaryViewMatrix, secondaryProjMatrix);

        Renderer::mvEndPass(get_current_command_buffer(GContext->graphics));

        //---------------------------------------------------------------------
        // primary pass
        //---------------------------------------------------------------------
        mvUpdateLightBuffers(am, light1, am.scenes[scene].asset.descriptorSet.descriptors[1].bufferID[GContext->graphics.currentImageIndex], viewMatrix, 1);
        mvUpdateLightBuffers(am, dlight1, am.scenes[scene].asset.descriptorSet.descriptors[2].bufferID[GContext->graphics.currentImageIndex], viewMatrix, 1);
        sceneData.camPos = camera.pos;
        mvBindScene(am, scene, sceneData, 1);
        Renderer::mvBeginPass(am, get_current_command_buffer(GContext->graphics), primaryPass);

        Renderer::mvRenderMesh(am, *light1.mesh, lightTransform, viewMatrix, projMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::mvRenderScene(am, am.scenes[i].asset, viewMatrix, projMatrix);

        if (sceneData.doSkybox)
        {
            mvBindSkybox(am, skybox);
            vkCmdBindPipeline(get_current_command_buffer(GContext->graphics), VK_PIPELINE_BIND_POINT_GRAPHICS, mvGetRawPipelineAsset(&am, "skybox_pass")->pipeline);
            Renderer::mvRenderSkybox(am, viewMatrix, projMatrix);
        }

        Renderer::mvEndPass(get_current_command_buffer(GContext->graphics));

        //---------------------------------------------------------------------
        // main pass
        //---------------------------------------------------------------------
        Renderer::mvBeginPass(am, get_current_command_buffer(GContext->graphics), mainPass);

        ImGui::SetNextWindowBgAlpha(1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); // to prevent main window corners from showing
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 5.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2((float)GContext->viewport.width, (float)GContext->viewport.height));

        static ImGuiWindowFlags windowFlags =
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoDecoration;

        ImGui::Begin("Main Window", 0, windowFlags);

        static ImGuiTableFlags tableflags =
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_SizingStretchProp |
            ImGuiTableFlags_NoHostExtendX;

        if (ImGui::BeginTable("Main Table", 3, tableflags))
        {

            ImGui::TableSetupColumn("Scene Controls", ImGuiTableColumnFlags_WidthFixed, 300.0f);
            ImGui::TableSetupColumn("Primary Scene");
            ImGui::TableSetupColumn("Light Controls", ImGuiTableColumnFlags_WidthFixed, 300.0f);
            ImGui::TableNextColumn();

            //-----------------------------------------------------------------------------
            // left panel
            //-----------------------------------------------------------------------------

            ImGui::TableSetColumnIndex(0);
            ImGui::Dummy(ImVec2(50.0f, 75.0f));

            ImGui::Checkbox("Diffuse Mapping", (bool*)&sceneData.doDiffuse);
            ImGui::Checkbox("Normal Mapping", (bool*)&sceneData.doNormal);
            ImGui::Checkbox("Specular Mapping", (bool*)&sceneData.doSpecular);
            ImGui::Checkbox("Omni Shadows", (bool*)&sceneData.doOmniShadows);
            ImGui::Checkbox("Direct Shadows", (bool*)&sceneData.doDirectionalShadows);
            ImGui::Checkbox("Skybox", (bool*)&sceneData.doSkybox);
            ImGui::Checkbox("PCF", (bool*)&sceneData.doPCF);
            ImGui::SliderInt("pcfRange", &sceneData.pcfRange, 1, 5);

            //-----------------------------------------------------------------------------
            // center panel
            //-----------------------------------------------------------------------------
            ImGui::TableSetColumnIndex(1);

            ImGuiIO& io = ImGui::GetIO();
            ImGui::GetForegroundDrawList()->AddText(ImVec2(ImGui::GetWindowPos().x + 45, 15),
                ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(io.Framerate) + " FPS").c_str());

            ImGui::GetForegroundDrawList()->AddText(ImVec2(ImGui::GetWindowPos().x + 45, 30),
                ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(camera.pos.x) + ", "
                    + std::to_string(camera.pos.y) + ", " + std::to_string(camera.pos.z)).c_str());
            ImGui::GetForegroundDrawList()->AddText(ImVec2(ImGui::GetWindowPos().x + 45, 45),
                ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(camera.pitch) + ", "
                    + std::to_string(camera.yaw)).c_str());

            ImVec2 contentSize = ImGui::GetContentRegionAvail();
            
            ImGui::Image(primaryPass.colorTextures[GContext->graphics.currentImageIndex].imguiID, contentSize);
            if (!(contentSize.x == oldContentRegion.x && contentSize.y == oldContentRegion.y))
            {
                primaryPass.viewport.width = contentSize.x;
                primaryPass.viewport.height = contentSize.y;
                camera.aspect = primaryPass.viewport.width / abs(primaryPass.viewport.height);
                recreatePrimaryRender = true;
            }
            oldContentRegion = contentSize;

            //-----------------------------------------------------------------------------
            // right panel
            //-----------------------------------------------------------------------------
            ImGui::TableSetColumnIndex(2);
            ImGui::Dummy(ImVec2(50.0f, 25.0f));
            ImGui::Indent(14.0f);
            ImGui::SliderFloat("omni depthBias", &omniShadowPass.specification.depthBias, 0.0f, 50.0f);
            ImGui::SliderFloat("omni slopeDepthBias", &omniShadowPass.specification.slopeDepthBias, 0.0f, 50.0f);
            ImGui::SliderFloat("directional depthBias", &shadowPass.specification.depthBias, 0.0f, 50.0f);
            ImGui::SliderFloat("directional slopeDepthBias", &shadowPass.specification.slopeDepthBias, 0.0f, 50.0f);
            if (ImGui::SliderFloat3("Position", &light1.info.worldPos.x, -50.0f, 50.0f))
            {
                lightTransform = mvTranslate(mvIdentityMat4(), light1.info.worldPos.xyz());
                lightcamera.pos = light1.info.worldPos.xyz();
            }

            if (ImGui::SliderFloat("Directional Light Angle", &angle, -45.0f, 45.0f))
            {
                zcomponent = sinf(MV_PI * angle / 180.0f);
                ycomponent = cosf(MV_PI * angle / 180.0f);

                secondaryCamera.dir = { 0.0f, -ycomponent, zcomponent };
                dlight1.info.viewLightDir = mvVec3{ 0.0, -ycomponent, zcomponent };
            }

            ImGui::Image(offscreenPass.colorTextures[GContext->graphics.currentImageIndex].imguiID, ImVec2(512, 512));
            ImGui::Unindent(14.0f);
            ImGui::EndTable();
            ImGui::End();
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(2);

        }

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), get_current_command_buffer(GContext->graphics));

        Renderer::mvEndPass(get_current_command_buffer(GContext->graphics));


        //---------------------------------------------------------------------
        // submit command buffers & present
        //---------------------------------------------------------------------
        Renderer::mvEndFrame();
        present(GContext->graphics);
    }

    Renderer::mvCleanupPass(primaryPass);
    mvCleanupAssetManager(&am);

    // cleanup imgui
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    cleanup_graphics_context(GContext->graphics);
    DestroyContext();
}