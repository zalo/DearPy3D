#include <imgui.h>
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

mv_internal const char* gltfModel = "FlightHelmet";
mv_internal const char* sponzaPath = "C:/dev/MarvelAssets/Sponza/";
mv_internal const char* gltfPath = "C://dev//glTF-Sample-Models//2.0//";
mv_internal b8 loadGLTF = false;
mv_internal b8 loadSponza = true;
mv_internal f32 shadowWidth = 2048.0f;
mv_internal f32 offscreenSize = 2048.0f;
mv_internal f32 offscreenWidth = 50.0f;

struct OffscreenSetup
{
    mvPass                 pass;
    mvPipeline             pipeline;
    std::vector<mvTexture> textures;
};

OffscreenSetup CreateSecondaryPass(mvAssetManager& am)
{
    std::vector<mvTexture> colorTexture;
    colorTexture.resize(GContext->graphics.swapChainImageViews.size());
    for (size_t i = 0; i < GContext->graphics.swapChainImageViews.size(); i++)
        colorTexture[i] = mvCreateTexture(
            offscreenSize, offscreenSize,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

    std::vector<mvTexture> depthTexture;
    depthTexture.resize(GContext->graphics.swapChainImageViews.size());
    for (size_t i = 0; i < GContext->graphics.swapChainImageViews.size(); i++)
        depthTexture[i] = mvCreateTexture(
            offscreenSize, offscreenSize,
            VK_FORMAT_D32_SFLOAT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_IMAGE_ASPECT_DEPTH_BIT);

    mvAssetID colorTextureIDs[3];
    colorTextureIDs[0] = mvRegisterAsset(&am, "offscreen1", colorTexture[0]);
    colorTextureIDs[1] = mvRegisterAsset(&am, "offscreen2", colorTexture[1]);
    colorTextureIDs[2] = mvRegisterAsset(&am, "offscreen3", colorTexture[2]);

    mvAssetID depthTextureIDs[3];
    depthTextureIDs[0] = mvRegisterAsset(&am, "doffscreen1", depthTexture[0]);
    depthTextureIDs[1] = mvRegisterAsset(&am, "doffscreen2", depthTexture[1]);
    depthTextureIDs[2] = mvRegisterAsset(&am, "doffscreen3", depthTexture[2]);

    VkRenderPass offscreenRenderPass = VK_NULL_HANDLE;

    {

        std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
        // Color attachment
        attchmentDescriptions[0].format = VK_FORMAT_R8G8B8A8_UNORM;
        attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        // Depth attachment
        attchmentDescriptions[1].format = VK_FORMAT_D32_SFLOAT;
        attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;
        subpassDescription.pDepthStencilAttachment = &depthReference;

        // Use subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = dependencies[0].dstStageMask;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = dependencies[0].dstAccessMask;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Create the actual renderpass
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
        renderPassInfo.pAttachments = attchmentDescriptions.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescription;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        vkCreateRenderPass(GContext->graphics.logicalDevice, &renderPassInfo, nullptr, &offscreenRenderPass);
        mvRegisterAsset(&am, "secondary_pass", offscreenRenderPass);
    }

    std::vector<VkFramebuffer> offscreenFramebuffers;
    offscreenFramebuffers.resize(GContext->graphics.swapChainImageViews.size());
    for (size_t i = 0; i < GContext->graphics.swapChainImageViews.size(); i++)
    {
        mvCreateFrameBuffer(offscreenRenderPass,
            offscreenFramebuffers[i],
            offscreenSize,
            offscreenSize,
            std::vector<VkImageView>{ colorTexture[i].imageInfo.imageView, depthTexture[i].imageInfo.imageView });
    }

    mvRegisterAsset(&am, "secondary_fb0", offscreenFramebuffers[0]);
    mvRegisterAsset(&am, "secondary_fb1", offscreenFramebuffers[1]);
    mvRegisterAsset(&am, "secondary_fb2", offscreenFramebuffers[2]);

    mvPipelineSpec pipelineSpec{};
    pipelineSpec.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineSpec.backfaceCulling = true;
    pipelineSpec.depthTest = true;
    pipelineSpec.depthWrite = true;
    pipelineSpec.wireFrame = false;
    pipelineSpec.vertexShader = "vs_shader.vert.spv";
    pipelineSpec.pixelShader = "ps_shader.frag.spv";
    pipelineSpec.width = offscreenSize;  // use viewport
    pipelineSpec.height = offscreenSize; // use viewport
    pipelineSpec.renderPass = offscreenRenderPass;
    pipelineSpec.layout = mvCreateVertexLayout(
        {
            mvVertexElementType::Position3D,
            mvVertexElementType::Normal,
            mvVertexElementType::Tangent,
            mvVertexElementType::Bitangent,
            mvVertexElementType::Texture2D
        });
    pipelineSpec.mainPass = false;

    mvPipeline offscreenPipeline = mvCreatePipeline(am, pipelineSpec);

    mvRegisterAsset(&am, "secondary_pass", offscreenPipeline);

    mvPass offscreenPass{
        offscreenRenderPass,
        GContext->graphics.swapChainExtent,
        offscreenFramebuffers
    };

    offscreenPass.viewport.x = 0.0f;
    offscreenPass.viewport.y = offscreenSize;
    offscreenPass.viewport.width = offscreenSize;
    offscreenPass.viewport.height = -offscreenSize;
    offscreenPass.viewport.minDepth = 0.0f;
    offscreenPass.viewport.maxDepth = 1.0f;
    offscreenPass.extent.width = (u32)offscreenSize;
    offscreenPass.extent.height = (u32)offscreenPass.viewport.y;

    OffscreenSetup offscreen{};
    offscreen.pass = offscreenPass;
    offscreen.pipeline = offscreenPipeline;
    offscreen.textures = colorTexture;
    return offscreen;
}

OffscreenSetup CreateShadowPass(mvAssetManager& am)
{

    std::vector<mvTexture> depthTexture;
    depthTexture.resize(GContext->graphics.swapChainImageViews.size());
    for (size_t i = 0; i < GContext->graphics.swapChainImageViews.size(); i++)
        depthTexture[i] = mvCreateTexture(
            shadowWidth, shadowWidth,
            VK_FORMAT_D32_SFLOAT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_DEPTH_BIT);

    mvAssetID depthTextureIDs[3];
    depthTextureIDs[0] = mvRegisterAsset(&am, "shadowoffscreen1", depthTexture[0]);
    depthTextureIDs[1] = mvRegisterAsset(&am, "shadowoffscreen2", depthTexture[1]);
    depthTextureIDs[2] = mvRegisterAsset(&am, "shadowoffscreen3", depthTexture[2]);

    VkRenderPass offscreenRenderPass = VK_NULL_HANDLE;

    {

        // Depth attachment
        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = VK_FORMAT_D32_SFLOAT;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// Clear depth at beginning of the render pass
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// We will read from depth, so it's important to store the depth attachment results
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// We don't care about initial layout of the attachment
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;// Attachment will be transitioned to shader read at render pass end

        VkAttachmentReference depthReference = {};
        depthReference.attachment = 0;
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;			// Attachment will be used as depth/stencil during render pass

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 0;													// No color attachments
        subpass.pDepthStencilAttachment = &depthReference;									// Reference to our depth attachment

        // Use subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Create the actual renderpass
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &attachmentDescription;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        vkCreateRenderPass(GContext->graphics.logicalDevice, &renderPassInfo, nullptr, &offscreenRenderPass);
        mvRegisterAsset(&am, "shadow_pass", offscreenRenderPass);
    }

    std::vector<VkFramebuffer> offscreenFramebuffers;
    offscreenFramebuffers.resize(GContext->graphics.swapChainImageViews.size());
    for (size_t i = 0; i < GContext->graphics.swapChainImageViews.size(); i++)
    {
        mvCreateFrameBuffer(offscreenRenderPass,
            offscreenFramebuffers[i],
            shadowWidth,
            shadowWidth,
            std::vector<VkImageView>{ depthTexture[i].imageInfo.imageView });
    }

    mvRegisterAsset(&am, "shadow_fb0", offscreenFramebuffers[0]);
    mvRegisterAsset(&am, "shadow_fb1", offscreenFramebuffers[1]);
    mvRegisterAsset(&am, "shadow_fb2", offscreenFramebuffers[2]);

    mvPipelineSpec pipelineSpec{};
    pipelineSpec.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineSpec.backfaceCulling = false;
    pipelineSpec.depthTest = true;
    pipelineSpec.depthWrite = true;
    pipelineSpec.wireFrame = false;
    pipelineSpec.vertexShader = "vs_shadow.vert.spv";
    //pipelineSpec.pixelShader = "ps_shadow.frag.spv";
    pipelineSpec.width = shadowWidth;  // use viewport
    pipelineSpec.height = shadowWidth; // use viewport
    pipelineSpec.renderPass = offscreenRenderPass;
    pipelineSpec.layout = mvCreateVertexLayout(
        {
            mvVertexElementType::Position3D,
            mvVertexElementType::Normal,
            mvVertexElementType::Tangent,
            mvVertexElementType::Bitangent,
            mvVertexElementType::Texture2D
        });
    pipelineSpec.mainPass = false;

    mvPipeline offscreenPipeline = mvCreatePipeline(am, pipelineSpec);

    mvRegisterAsset(&am, "shadow_pass", offscreenPipeline);

    mvPass offscreenPass{
        offscreenRenderPass,
        GContext->graphics.swapChainExtent,
        offscreenFramebuffers
            };

    offscreenPass.viewport.x = 0.0f;
    offscreenPass.viewport.y = shadowWidth;
    offscreenPass.viewport.width = shadowWidth;
    offscreenPass.viewport.height = -shadowWidth;
    offscreenPass.viewport.minDepth = 0.0f;
    offscreenPass.viewport.maxDepth = 1.0f;
    offscreenPass.extent.width = (u32)shadowWidth;
    offscreenPass.extent.height = (u32)offscreenPass.viewport.y;

    OffscreenSetup offscreen{};
    offscreen.pass = offscreenPass;
    offscreen.pipeline = offscreenPipeline;
    return offscreen;
}

int main() 
{

    CreateContext();
    
    mvInitializeViewport(500, 500);
    GContext->IO.shaderDirectory = "../../DearPy3D/shaders/";
    GContext->graphics.enableValidationLayers = true;
    GContext->graphics.validationLayers = { "VK_LAYER_KHRONOS_validation" };
    GContext->graphics.deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    mvSetupGraphicsContext();

    mvAssetManager am{};
    mvInitializeAssetManager(&am);

    mvAssetID scene = mvRegisterAsset(&am, "test_scene", mvCreateScene(am, {}));

    if (loadSponza) mvLoadOBJAssets(am, sponzaPath, "sponza");

    mvSceneData sceneData{};

    mvCamera camera{};
    camera.pos = { -13.5f, 6.0f, 3.5f };
    camera.front = { 0.0f, 0.0f, 1.0f };
    camera.up = { 0.0f, -1.0f, 0.0f };
    camera.pitch = 0.0f;
    camera.yaw = 0.0f;
    camera.aspect = GContext->viewport.width / GContext->viewport.height;

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
    
    mvMesh quad1 = mvCreateTexturedQuad(am);
    mvRegisterAsset(&am, "quad1", quad1);
    mvMat4 quadTransform = mvTranslate(mvIdentityMat4(), mvVec3{ 5.0f, 5.0f, 5.0f });

    mvMesh cube1 = mvCreateTexturedCube(am, 3.0f);
    mvRegisterAsset(&am, "cube1", cube1);
    mvMat4 cubeTransform = mvTranslate(mvIdentityMat4(), mvVec3{ 10.0f, 10.0f, 20.0f });

    mvPointLight light1 = mvCreatePointLight(am, "light1", { 0.0f, 10.0f, 0.0f });
    mvMat4 lightTransform = mvTranslate(mvIdentityMat4(), mvVec3{ 0.0f, 10.0f, 0.0f });
    mvDirectionLight dlight1 = mvCreateDirectionLight(am, "dlight1", mvVec3{ 0.0, -1.0f, 0.0f });
   
    //---------------------------------------------------------------------
    // passes
    //---------------------------------------------------------------------

    mvPass mainPass{
        GContext->graphics.renderPass,
        GContext->graphics.swapChainExtent,
        GContext->graphics.swapChainFramebuffers
    };

    mainPass.viewport.x = 0.0f;
    mainPass.viewport.y = GContext->graphics.swapChainExtent.height;
    mainPass.viewport.width = GContext->graphics.swapChainExtent.width;
    mainPass.viewport.height = -(int)GContext->graphics.swapChainExtent.height;
    mainPass.viewport.minDepth = 0.0f;
    mainPass.viewport.maxDepth = 1.0f;
    mainPass.extent.width = (u32)GContext->graphics.swapChainExtent.width;
    mainPass.extent.height = (u32)mainPass.viewport.y;

    OffscreenSetup offscreen = CreateSecondaryPass(am);
    OffscreenSetup shadow = CreateShadowPass(am);

    mvAssetID depthIDs[3];
    depthIDs[0] = mvGetTextureAssetID(&am, "shadowoffscreen1");
    depthIDs[1] = mvGetTextureAssetID(&am, "shadowoffscreen2");
    depthIDs[2] = mvGetTextureAssetID(&am, "shadowoffscreen3");

    VkPipeline mainPipeline = mvGetRawPipelineAsset(&am, "main_pass")->pipeline;

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
            mvPrepareResizeAssetManager(&am);

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

            mainPipeline = mvGetRawPipelineAsset(&am, "main_pass")->pipeline;
        }

        //---------------------------------------------------------------------
        // input handling
        //---------------------------------------------------------------------
        mvUpdateCameraFPSCamera(camera, dt, 12.0f, 1.0f);

        mvMat4 viewMatrix = mvCreateFPSView(camera);
        mvMat4 projMatrix = mvCreateLookAtProjection(camera);

        mvMat4 secondaryViewMatrix = mvCreateOrthoView(secondaryCamera);
        mvMat4 secondaryProjMatrix = mvCreateOrthoProjection(secondaryCamera);
        sceneData.directionalShadowView = secondaryViewMatrix;
        sceneData.directionalShadowProjection = secondaryProjMatrix;
 
        //---------------------------------------------------------------------
        // wait for fences and acquire next image
        //---------------------------------------------------------------------
        Renderer::mvBeginFrame();
        mvUpdateSceneDescriptors(am, am.scenes[scene].asset, depthIDs[GContext->graphics.currentImageIndex]);
        Renderer::mvUpdateDescriptors(am);
        
        mvShowAssetManager(am);

        ImGuiIO& io = ImGui::GetIO();
        ImGui::GetForegroundDrawList()->AddText(ImVec2(45, 45),
            ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(io.Framerate) + " FPS").c_str());

        if (ImGui::Begin("Debug Output", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            ImGui::Image(offscreen.textures[GContext->graphics.currentImageIndex].imguiID, ImVec2(512, 512));
        ImGui::End();

        ImGui::Begin("Light Controls");
        mv_local_persist f32 depthBias = 50.0f;
        mv_local_persist f32 slopeDepthBias = 1.75f;
        ImGui::SliderFloat("depthBias", &depthBias, -50.0f, 50.0f);
        ImGui::SliderFloat("slopeDepthBias", &slopeDepthBias, -50.0f, 50.0f);
        if (ImGui::SliderFloat3("Position", &light1.info.viewLightPos.x, -50.0f, 50.0f))
        {
            lightTransform = mvTranslate(mvIdentityMat4(), light1.info.viewLightPos.xyz());
        }
        ImGui::End();

        ImGui::Begin("Scene");
        ImGui::Checkbox("Diffuse Mapping", (bool*)&sceneData.doDiffuse);
        ImGui::Checkbox("Normal Mapping", (bool*)&sceneData.doNormal);
        ImGui::Checkbox("Specular Mapping", (bool*)&sceneData.doSpecular);
        ImGui::Checkbox("Omni Shadows", (bool*)&sceneData.doOmniShadows);
        ImGui::Checkbox("Direct Shadows", (bool*)&sceneData.doDirectionalShadows);
        ImGui::Checkbox("Skybox", (bool*)&sceneData.doSkybox);
        ImGui::Checkbox("PCF", (bool*)&sceneData.doPCF);
        ImGui::SliderInt("pcfRange", &sceneData.pcfRange, 1, 5);
        ImGui::End();

        //---------------------------------------------------------------------
        // shadow pass
        //---------------------------------------------------------------------
        Renderer::mvBeginDepthOnlyPass(mvGetCurrentCommandBuffer(), shadow.pass);

        vkCmdSetDepthBias(
            mvGetCurrentCommandBuffer(),
            depthBias,
            0.0f,
            slopeDepthBias);

        vkCmdBindPipeline(mvGetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, shadow.pipeline.pipeline);

        Renderer::mvRenderMeshShadow(am, *light1.mesh, lightTransform, secondaryViewMatrix, secondaryProjMatrix);
        Renderer::mvRenderMeshShadow(am, cube1, cubeTransform, secondaryViewMatrix, secondaryProjMatrix);
        Renderer::mvRenderMeshShadow(am, quad1, quadTransform, secondaryViewMatrix, secondaryProjMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::mvRenderSceneShadow(am, am.scenes[i].asset, secondaryViewMatrix, secondaryProjMatrix);

        Renderer::mvEndPass(mvGetCurrentCommandBuffer());

        //---------------------------------------------------------------------
        // offscreen pass
        //---------------------------------------------------------------------
        
        mvUpdateLightBuffers(am, light1, am.scenes[scene].asset.descriptorSet.descriptors[1].bufferID[GContext->graphics.currentImageIndex], secondaryViewMatrix, 0);
        mvUpdateLightBuffers(am, dlight1, am.scenes[scene].asset.descriptorSet.descriptors[2].bufferID[GContext->graphics.currentImageIndex], secondaryViewMatrix, 0);
        mvBindScene(am, scene, sceneData, 0);
        Renderer::mvBeginPass(mvGetCurrentCommandBuffer(), offscreen.pass);

        vkCmdBindPipeline(mvGetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, offscreen.pipeline.pipeline);

        Renderer::mvRenderMesh(am, *light1.mesh, lightTransform, secondaryViewMatrix, secondaryProjMatrix);
        Renderer::mvRenderMesh(am, cube1, cubeTransform, secondaryViewMatrix, secondaryProjMatrix);
        Renderer::mvRenderMesh(am, quad1, quadTransform, secondaryViewMatrix, secondaryProjMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::mvRenderScene(am, am.scenes[i].asset, secondaryViewMatrix, secondaryProjMatrix);

        Renderer::mvEndPass(mvGetCurrentCommandBuffer());

        //---------------------------------------------------------------------
        // main pass
        //---------------------------------------------------------------------
        mvUpdateLightBuffers(am, light1, am.scenes[scene].asset.descriptorSet.descriptors[1].bufferID[GContext->graphics.currentImageIndex], viewMatrix, 1);
        mvUpdateLightBuffers(am, dlight1, am.scenes[scene].asset.descriptorSet.descriptors[2].bufferID[GContext->graphics.currentImageIndex], viewMatrix, 1);
        mvBindScene(am, scene, sceneData, 1);
        Renderer::mvBeginPass(mvGetCurrentCommandBuffer(), mainPass);

        vkCmdBindPipeline(mvGetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mainPipeline);

        Renderer::mvRenderMesh(am, *light1.mesh, lightTransform, viewMatrix, projMatrix);
        Renderer::mvRenderMesh(am, cube1, cubeTransform, viewMatrix, projMatrix);
        Renderer::mvRenderMesh(am, quad1, quadTransform, viewMatrix, projMatrix);

        for (int i = 0; i < am.sceneCount; i++)
            Renderer::mvRenderScene(am, am.scenes[i].asset, viewMatrix, projMatrix);

        ImGui::Render();
        mvRecordImGui(mvGetCurrentCommandBuffer());

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