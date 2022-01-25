#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <array>
#include "mvMesh.h"
#include "mvCamera.h"
#include "mvViewport.h"
#include "mvGraphics.h"
#include "mvTimer.h"
#include "mvLights.h"
#include "mvMaterials.h"
#include <iostream>
#include "mvMath.h"
#include "mvRenderer.h"
#include "mvObjLoader.h"
#include "mvAssetManager.h"
#include "mvScene.h"
#include "mvSkybox.h"
#include <stdlib.h>

#include "passes.h"

mv_internal const char* sponzaPath = "C:/dev/MarvelAssets/Sponza/";
mv_internal b8 loadSponza = true;
mv_internal ImVec2 oldContentRegion = ImVec2(500, 500);

int main() 
{
    int result = putenv("VK_LAYER_PATH=..\\..\\Dependencies\\vk_sdk_lite\\Bin");
    
    mvViewport viewport{};
    initialize_viewport(viewport, 500, 500);

    mvGraphics graphics{};
    graphics.shaderDirectory = "../../DearPy3D/shaders/";
    graphics.enableValidationLayers = true;


    setup_graphics_context(graphics, viewport, { "VK_LAYER_KHRONOS_validation" }, { VK_KHR_SWAPCHAIN_EXTENSION_NAME });

    mvAssetManager am{};
    mvInitializeAssetManager(&am);

    mvPipelineManager pipelineManager = create_pipeline_manager();
    mvDescriptorManager descriptorManager = create_descriptor_manager();
    mvMaterialManager materialManager = create_material_manager();

    //---------------------------------------------------------------------
    // load common descriptor sets
    //---------------------------------------------------------------------
    VkDescriptorSetLayout descriptorSetLayouts[4];
    {

        mvDescriptorSetLayout globalLayout = create_descriptor_set_layout(graphics,
            {
                create_dynamic_uniform_descriptor_spec(0u),
                create_dynamic_uniform_descriptor_spec(1u),
                create_dynamic_uniform_descriptor_spec(2u),
                create_texture_descriptor_spec(3u),
                create_texture_descriptor_spec(4u)
            });

        mvDescriptorSetLayout materialLayout = create_descriptor_set_layout(graphics,
            {
                create_texture_descriptor_spec(0u),
                create_texture_descriptor_spec(1u),
                create_texture_descriptor_spec(2u),
                create_uniform_descriptor_spec(3u)
            });


        mvDescriptorSetLayout perDrawLayout = create_descriptor_set_layout(graphics,
            {
                create_dynamic_uniform_descriptor_spec(0u)
            });

        mvDescriptorSetLayout skyboxLayout = create_descriptor_set_layout(graphics,
            {
                create_texture_descriptor_spec(0u)
            });

        descriptorSetLayouts[0] = globalLayout.layout;
        descriptorSetLayouts[1] = materialLayout.layout;
        descriptorSetLayouts[2] = perDrawLayout.layout;
        descriptorSetLayouts[3] = skyboxLayout.layout;

        register_descriptor_set_layout(descriptorManager, "scene", descriptorSetLayouts[0]);
        register_descriptor_set_layout(descriptorManager, "phong", descriptorSetLayouts[1]);
        register_descriptor_set_layout(descriptorManager, "perdraw", descriptorSetLayouts[2]);
        register_descriptor_set_layout(descriptorManager, "skybox_pass", descriptorSetLayouts[3]);
    }

    //---------------------------------------------------------------------
    // load common pipelines
    //---------------------------------------------------------------------
    
    { // main pass

        VkPipelineLayout pipelineLayout;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;

        MV_VULKAN(vkCreatePipelineLayout(graphics.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout));

        register_pipeline_layout(pipelineManager, "main_pass", pipelineLayout);
    }

    { // primary pass
        
        VkPipelineLayout pipelineLayout;
        VkPushConstantRange push_constant;
        push_constant.offset = 0;
        push_constant.size = 192;
        push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 3;
        pipelineLayoutInfo.pPushConstantRanges = &push_constant;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;

        MV_VULKAN(vkCreatePipelineLayout(graphics.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout));

        mvAssetID pipelineLayoutID = register_pipeline_layout(pipelineManager, "primary_pass", pipelineLayout);

        mvTransforms* transforms = new mvTransforms[am.maxMeshCount * 3];
        mvDescriptorSet descriptorSet = create_descriptor_set(graphics, descriptorSetLayouts[2], pipelineLayout);
        descriptorSet.descriptors.push_back(create_dynamic_uniform_descriptor(graphics, create_dynamic_uniform_descriptor_spec(0u), "transforms", am.maxMeshCount * 3, sizeof(mvTransforms), transforms));
        register_descriptor_set(descriptorManager, "perdraw", descriptorSet);
        delete[] transforms;
    }

    { // shadow pass

        VkPipelineLayout pipelineLayout;
        VkPushConstantRange push_constant;
        push_constant.offset = 0;
        push_constant.size = 192;
        push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = &push_constant;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pSetLayouts = nullptr;

        MV_VULKAN(vkCreatePipelineLayout(graphics.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout));

        register_pipeline_layout(pipelineManager, "shadow_pass", pipelineLayout);
    }

    { // skybox pass

        VkPipelineLayout pipelineLayout;
        VkPushConstantRange push_constant;
        push_constant.offset = 0;
        push_constant.size = 64;
        push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &push_constant;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayouts[3];

        MV_VULKAN(vkCreatePipelineLayout(graphics.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout));

        register_pipeline_layout(pipelineManager, "skybox_pass", pipelineLayout);
    }

    { // omni shadow pass

        VkPipelineLayout pipelineLayout;
        VkPushConstantRange push_constant;
        push_constant.offset = 0;
        push_constant.size = 80;
        push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = &push_constant;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pSetLayouts = nullptr;

        MV_VULKAN(vkCreatePipelineLayout(graphics.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout));

        register_pipeline_layout(pipelineManager, "omnishadow_pass", pipelineLayout);
    }

    mvAssetID scene = mvRegisterAsset(&am, "test_scene", create_scene(graphics, am, descriptorManager, pipelineManager, {}));

    if (loadSponza) load_obj_assets(graphics, am, descriptorManager, pipelineManager, materialManager, sponzaPath, "sponza");

    mvSkybox skybox = create_skybox(graphics, am, materialManager, descriptorManager, pipelineManager);

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
    
    mvPointLight light1 = create_point_light(graphics, am, descriptorManager, pipelineManager, materialManager, "light1", { 40.0f, 10.0f, 0.0f });
    mvMat4 lightTransform = translate( 40.0f, 10.0f, 0.0f );
    mvDirectionLight dlight1 = create_directional_light(am, "dlight1", mvVec3{ 0.0, -ycomponent, zcomponent });

    mvCamera lightcamera{};
    lightcamera.pos = light1.info.worldPos;
    lightcamera.fieldOfView = M_PI_2;
   
    // passes
    mvPass mainPass = create_main_pass(graphics, am);
    mvPass primaryPass = create_primary_pass(graphics, am, descriptorManager, pipelineManager, oldContentRegion.x, oldContentRegion.y);
    mvPass offscreenPass = create_offscreen_pass(graphics, am, descriptorManager, pipelineManager);
    mvPass shadowPass = create_shadow_pass(graphics, am, descriptorManager, pipelineManager);
    mvPass omniShadowPass = create_omnishadow_pass(graphics, am, descriptorManager, pipelineManager);

    mvTexture shadowMapCube = create_texture_cube(graphics,
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

    VkPipelineLayout primaryLayout = get_pipeline_layout(pipelineManager, "primary_pass");
    VkPipelineLayout shadowPLayout = get_pipeline_layout(pipelineManager, "shadow_pass");
    VkPipelineLayout skyboxLayout = get_pipeline_layout(pipelineManager, "skybox_pass");

    while (viewport.running)
    {
        const auto dt = timer.mark() * 1.0f;

        process_viewport_events(viewport);

        //---------------------------------------------------------------------
        // handle window resizing
        //---------------------------------------------------------------------
        if (viewport.resized)
        {

            int newwidth = 0, newheight = 0;
            glfwGetFramebufferSize(viewport.handle, &newwidth, &newheight);
            while (newwidth == 0 || newheight == 0)
            {
                glfwGetFramebufferSize(viewport.handle, &newwidth, &newheight);
                glfwWaitEvents();
            }

            // cleanup
            viewport.width = newwidth;
            viewport.height = newheight;
            recreate_swapchain(graphics, viewport);
            mainPass = create_main_pass(graphics, am);
            viewport.resized = false;
        }

        if (recreatePrimaryRender)
        {
            Renderer::cleanup_pass(graphics, primaryPass);
            primaryPass = create_primary_pass(graphics, am, descriptorManager, pipelineManager, primaryPass.viewport.width, abs(primaryPass.viewport.height));
            recreatePrimaryRender = false;
            primaryLayout = get_pipeline_layout(pipelineManager, "primary_pass");
        }

        //---------------------------------------------------------------------
        // input handling
        //---------------------------------------------------------------------
        update_fps_camera(viewport, camera, dt, 12.0f, 1.0f);

        mvMat4 viewMatrix = fps_view(camera.pos, camera.pitch, camera.yaw);
        mvMat4 projMatrix = perspective(camera.fieldOfView, camera.aspect, camera.nearZ, camera.farZ);
        
        mvMat4 secondaryViewMatrix = look_at(secondaryCamera.pos, secondaryCamera.pos - secondaryCamera.dir, secondaryCamera.up);
        mvMat4 secondaryProjMatrix = orthographic( secondaryCamera.left, secondaryCamera.right, secondaryCamera.bottom,
            secondaryCamera.top, secondaryCamera.nearZ, secondaryCamera.farZ);
        {
            mvVec3 direction{};
            direction.x = cos((lightcamera.yaw)) * cos((lightcamera.pitch));
            direction.y = sin((lightcamera.pitch));
            direction.z = sin((lightcamera.yaw)) * cos((lightcamera.pitch));
            direction = normalize(direction);
            sceneData.pointShadowView = look_at(lightcamera.pos, lightcamera.pos + direction, lightcamera.up);
        }
        sceneData.directionalShadowView = secondaryViewMatrix;
        sceneData.directionalShadowProjection = secondaryProjMatrix;
 
        //---------------------------------------------------------------------
        // wait for fences and acquire next image
        //---------------------------------------------------------------------
        Renderer::begin_frame(graphics);
        update_skybox_descriptors(graphics, am, skybox, mvGetTextureAssetID2(graphics, &am, "../../Resources/SkyBox"));
        update_scene_descriptors(graphics, am, am.scenes[scene].asset,
            shadowPass.depthTextures[graphics.currentImageIndex],cube);
        Renderer::update_descriptors(graphics, am, materialManager);
        
        //---------------------------------------------------------------------
        // shadow pass
        //---------------------------------------------------------------------
        Renderer::begin_pass(graphics, pipelineManager, get_current_command_buffer(graphics), shadowPass);

        
        Renderer::render_mesh_shadow(graphics, shadowPLayout ,*light1.mesh, lightTransform, secondaryViewMatrix, secondaryProjMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::render_scene_shadows(graphics, am, shadowPLayout, am.scenes[i].asset, secondaryViewMatrix, secondaryProjMatrix);

        Renderer::end_pass(get_current_command_buffer(graphics));

        //---------------------------------------------------------------------
        // omni shadow pass
        //---------------------------------------------------------------------
        for (u32 i = 0; i < 6; i++)
        {

            mvMat4 camera_matrix(1.0f);
            mvVec3 look_target{};
            switch (i)
            {
            case 0: // POSITIVE_X
                look_target = light1.info.worldPos + mvVec3{ 0.0f, 0.0f, 1.0f };
                camera_matrix = look_at(light1.info.worldPos, look_target, mvVec3{ 0.0f, 1.0f, 0.0f });
                break;
            case 1:	// NEGATIVE_X
                look_target = light1.info.worldPos + mvVec3{ 0.0f, 0.0f, -1.0f };
                camera_matrix = look_at(light1.info.worldPos, look_target, mvVec3{ 0.0f, 1.0f, 0.0f });
                break;
            case 2:	// POSITIVE_Y
                look_target = light1.info.worldPos + mvVec3{ 0.0f, -1.0f, 0.0f };
                camera_matrix = look_at(light1.info.worldPos, look_target, mvVec3{ 1.0f, 0.0f, 0.0f });
                break;
            case 3:	// NEGATIVE_Y
                look_target = light1.info.worldPos + mvVec3{ 0.0f, 1.0f, 0.0f };
                camera_matrix = look_at(light1.info.worldPos, look_target, mvVec3{ -1.0f, 0.0f, 0.0f });
                break;
            case 4:	// POSITIVE_Z
                look_target = light1.info.worldPos+ mvVec3{ 1.0f, 0.0f, 0.0f };
                camera_matrix = look_at(light1.info.worldPos, look_target, mvVec3{ 0.0f, 1.0f, 0.0f });
                break;
            case 5:	// NEGATIVE_Z
                look_target = light1.info.worldPos+ mvVec3{ -1.0f, 0.0f, 0.0f };
                camera_matrix = look_at(light1.info.worldPos, look_target, mvVec3{ 0.0f, 1.0f, 0.0f });
                break;
            }

            Renderer::begin_pass(graphics, pipelineManager, get_current_command_buffer(graphics), omniShadowPass);

            for (int i = 0; i < am.sceneCount; i++)
                Renderer::render_scene_omni_shadows(graphics, am, shadowPLayout, am.scenes[i].asset, camera_matrix, perspective(M_PI_2, 1.0f, 0.1f, 400.0f), light1.info.worldPos);

            Renderer::end_pass(get_current_command_buffer(graphics));

            transition_image_layout(get_current_command_buffer(graphics),
                omniShadowPass.colorTextures[graphics.currentImageIndex].textureImage,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

            VkImageSubresourceRange cubeFaceSubresourceRange = {};
            cubeFaceSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            cubeFaceSubresourceRange.baseMipLevel = 0;
            cubeFaceSubresourceRange.levelCount = 1;
            cubeFaceSubresourceRange.baseArrayLayer = i;
            cubeFaceSubresourceRange.layerCount = 1;

            transition_image_layout(get_current_command_buffer(graphics),
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
                get_current_command_buffer(graphics),
                omniShadowPass.colorTextures[graphics.currentImageIndex].textureImage,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                shadowMapCube.textureImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &copyRegion);

            transition_image_layout(get_current_command_buffer(graphics),
                omniShadowPass.colorTextures[graphics.currentImageIndex].textureImage,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

            transition_image_layout(get_current_command_buffer(graphics),
                shadowMapCube.textureImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                cubeFaceSubresourceRange);
        }

        //---------------------------------------------------------------------
        // offscreen pass
        //--------------------------------------------------------------------- 
        update_light_buffers(graphics, light1, am.scenes[scene].asset.descriptorSet.descriptors[1].buffers[graphics.currentImageIndex], secondaryViewMatrix, 0);
        update_light_buffers(graphics, dlight1, am.scenes[scene].asset.descriptorSet.descriptors[2].buffers[graphics.currentImageIndex], secondaryViewMatrix, 0);
        sceneData.camPos = secondaryCamera.pos;
        bind_scene(graphics, am.scenes[scene].asset, sceneData, 0);
        
        Renderer::begin_pass(graphics, pipelineManager, get_current_command_buffer(graphics), offscreenPass);

        Renderer::render_mesh(graphics, materialManager.materials[light1.mesh->phongMaterialID].descriptorSet, primaryLayout, *light1.mesh, lightTransform, secondaryViewMatrix, secondaryProjMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::render_scene(graphics, am, materialManager, primaryLayout, am.scenes[i].asset, secondaryViewMatrix, secondaryProjMatrix);

        Renderer::end_pass(get_current_command_buffer(graphics));

        //---------------------------------------------------------------------
        // primary pass
        //---------------------------------------------------------------------
        update_light_buffers(graphics, light1, am.scenes[scene].asset.descriptorSet.descriptors[1].buffers[graphics.currentImageIndex], viewMatrix, 1);
        update_light_buffers(graphics, dlight1, am.scenes[scene].asset.descriptorSet.descriptors[2].buffers[graphics.currentImageIndex], viewMatrix, 1);
        sceneData.camPos = camera.pos;
        bind_scene(graphics, am.scenes[scene].asset, sceneData, 1);
        Renderer::begin_pass(graphics, pipelineManager, get_current_command_buffer(graphics), primaryPass);

        Renderer::render_mesh(graphics, materialManager.materials[light1.mesh->phongMaterialID].descriptorSet, primaryLayout, *light1.mesh, lightTransform, viewMatrix, projMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::render_scene(graphics, am, materialManager, primaryLayout, am.scenes[i].asset, viewMatrix, projMatrix);

        if (sceneData.doSkybox)
        {
            bind_skybox(graphics, skybox);
            vkCmdBindPipeline(get_current_command_buffer(graphics), VK_PIPELINE_BIND_POINT_GRAPHICS, get_pipeline(pipelineManager, "skybox_pass"));
            Renderer::render_skybox(skybox, graphics, skyboxLayout, viewMatrix, projMatrix);
        }

        Renderer::end_pass(get_current_command_buffer(graphics));

        //---------------------------------------------------------------------
        // main pass
        //---------------------------------------------------------------------
        Renderer::begin_pass(graphics, pipelineManager, get_current_command_buffer(graphics), mainPass);

        ImGui::SetNextWindowBgAlpha(1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); // to prevent main window corners from showing
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 5.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2((float)viewport.width, (float)viewport.height));

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
            
            ImGui::Image(primaryPass.colorTextures[graphics.currentImageIndex].imguiID, contentSize);
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
                lightTransform = translate(light1.info.worldPos.x, light1.info.worldPos.y, light1.info.worldPos.z);
                lightcamera.pos = light1.info.worldPos;
            }

            if (ImGui::SliderFloat("Directional Light Angle", &angle, -45.0f, 45.0f))
            {
                zcomponent = sinf(MV_PI * angle / 180.0f);
                ycomponent = cosf(MV_PI * angle / 180.0f);

                secondaryCamera.dir = { 0.0f, -ycomponent, zcomponent };
                dlight1.info.viewLightDir = mvVec3{ 0.0, -ycomponent, zcomponent };
            }

            ImGui::Image(offscreenPass.colorTextures[graphics.currentImageIndex].imguiID, ImVec2(512, 512));
            ImGui::Unindent(14.0f);
            ImGui::EndTable();
            ImGui::End();
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(2);

        }

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), get_current_command_buffer(graphics));

        Renderer::end_pass(get_current_command_buffer(graphics));


        //---------------------------------------------------------------------
        // submit command buffers & present
        //---------------------------------------------------------------------
        Renderer::end_frame(graphics);
        present(graphics);
    }

    Renderer::cleanup_pass(graphics, primaryPass);
    Renderer::cleanup_pass(graphics, offscreenPass);
    Renderer::cleanup_pass(graphics, omniShadowPass);
    Renderer::cleanup_pass(graphics, shadowPass);
    cleanup_descriptor_manager(graphics, descriptorManager);
    cleanup_pipeline_manager(graphics, pipelineManager);
    cleanup_material_manager(graphics, materialManager);

    vkDestroyBuffer(graphics.logicalDevice, skybox.mesh.indexBuffer.buffer, nullptr);
    vkDestroyBuffer(graphics.logicalDevice, skybox.mesh.vertexBuffer.buffer, nullptr);
    vkFreeMemory(graphics.logicalDevice, skybox.mesh.indexBuffer.deviceMemory, nullptr);
    vkFreeMemory(graphics.logicalDevice, skybox.mesh.vertexBuffer.deviceMemory, nullptr);
    skybox.mesh.indexBuffer.buffer = VK_NULL_HANDLE;
    skybox.mesh.indexBuffer.deviceMemory = VK_NULL_HANDLE;
    skybox.mesh.indexBuffer.specification.count = 0u;
    skybox.mesh.indexBuffer.specification.aligned = false;
    skybox.mesh.vertexBuffer.buffer = VK_NULL_HANDLE;
    skybox.mesh.vertexBuffer.deviceMemory = VK_NULL_HANDLE;
    skybox.mesh.vertexBuffer.specification.count = 0u;
    skybox.mesh.vertexBuffer.specification.aligned = false;

    mvCleanupAssetManager(graphics, &am);

    // cleanup imgui
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    cleanup_graphics_context(graphics);
    glfwTerminate();
}