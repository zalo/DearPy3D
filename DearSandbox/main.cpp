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
mv_internal f32 shadowWidth = 100.0f;

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
            512, 512,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

    std::vector<mvTexture> depthTexture;
    depthTexture.resize(GContext->graphics.swapChainImageViews.size());
    for (size_t i = 0; i < GContext->graphics.swapChainImageViews.size(); i++)
        depthTexture[i] = mvCreateTexture(
            512, 512,
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
            512,
            512,
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
    pipelineSpec.vertexShader = "vs_basic.vert.spv";
    pipelineSpec.pixelShader = "ps_basic.frag.spv";
    pipelineSpec.width = (float)512.0f;  // use viewport
    pipelineSpec.height = (float)512.0f; // use viewport
    pipelineSpec.renderPass = offscreenRenderPass;
    pipelineSpec.layout = mvCreateVertexLayout({});
    pipelineSpec.layout.bindingDescriptions.clear();
    pipelineSpec.mainPass = false;

    mvPipeline offscreenPipeline = mvCreatePipeline(am, pipelineSpec);

    mvRegisterAsset(&am, "secondary_pass", offscreenPipeline);

    mvPass offscreenPass{
        offscreenRenderPass,
        GContext->graphics.swapChainExtent,
        offscreenFramebuffers
            };

    offscreenPass.viewport.x = 0.0f;
    offscreenPass.viewport.y = 512;
    offscreenPass.viewport.width = 512;
    offscreenPass.viewport.height = -512;
    offscreenPass.viewport.minDepth = 0.0f;
    offscreenPass.viewport.maxDepth = 1.0f;
    offscreenPass.extent.width = (u32)512;
    offscreenPass.extent.height = (u32)offscreenPass.viewport.y;

    OffscreenSetup offscreen{};
    offscreen.pass = offscreenPass;
    offscreen.pipeline = offscreenPipeline;
    offscreen.textures = colorTexture;
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

    mvCamera camera{};
    camera.pos = { -13.5f, 6.0f, 3.5f };
    camera.front = { 0.0f, 0.0f, 1.0f };
    camera.up = { 0.0f, -1.0f, 0.0f };
    camera.pitch = 0.0f;
    camera.yaw = 0.0f;
    camera.aspect = GContext->viewport.width / GContext->viewport.height;
    
    mvMesh quad1 = mvCreateTexturedQuad(am);
    mvRegisterAsset(&am, "quad1", quad1);
    mvMat4 quadTransform = mvTranslate(mvIdentityMat4(), mvVec3{ 5.0f, 5.0f, 5.0f });

    mvMesh cube1 = mvCreateTexturedCube(am, 3.0f);
    mvRegisterAsset(&am, "cube1", cube1);
    mvMat4 cubeTransform = mvTranslate(mvIdentityMat4(), mvVec3{ 10.0f, 10.0f, 20.0f });

    mvPointLight light = mvCreatePointLight(am, { 0.0f, 10.0f, 0.0f });
    mvMat4 lightTransform = mvTranslate(mvIdentityMat4(), mvVec3{ 0.0f, 10.0f, 0.0f });

    mvDirectionLight dlight = mvCreateDirectionLight(am, { 0.0, -1.0f, 0.0f });
    
    mvUpdateSceneDescriptors(am, am.scenes[scene].asset, light, dlight);

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
        }

        //---------------------------------------------------------------------
        // input handling
        //---------------------------------------------------------------------
        mvUpdateCameraFPSCamera(camera, dt, 12.0f, 1.0f);
 
        //---------------------------------------------------------------------
        // wait for fences and acquire next image
        //---------------------------------------------------------------------
        Renderer::mvBeginFrame();

        mvBindScene(am, scene);
        Renderer::mvUpdateDescriptors(am);

        //---------------------------------------------------------------------
        // offscreen pass
        //---------------------------------------------------------------------

        Renderer::mvBeginPass(mvGetCurrentCommandBuffer(), offscreen.pass);

        vkCmdBindPipeline(mvGetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, offscreen.pipeline.pipeline);
        vkCmdDraw(mvGetCurrentCommandBuffer(), 3, 1, 0, 0);

        Renderer::mvEndPass(mvGetCurrentCommandBuffer());

        //---------------------------------------------------------------------
        // main pass
        //---------------------------------------------------------------------

        Renderer::mvBeginPass(mvGetCurrentCommandBuffer(), mainPass);

        ImGuiIO& io = ImGui::GetIO();
        ImGui::GetForegroundDrawList()->AddText(ImVec2(45, 45),
            ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(io.Framerate) + " FPS").c_str());

        mvShowAssetManager(am);

        if (ImGui::Begin("Debug Output", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            ImGui::Image(offscreen.textures[GContext->graphics.currentImageIndex].imguiID, ImVec2(512, 512));
        ImGui::End();

        ImGui::Begin("Light Controls");
        if (ImGui::SliderFloat3("Position", &light.info.viewLightPos.x, -25.0f, 25.0f))
        {
            lightTransform = mvTranslate(mvIdentityMat4(), light.info.viewLightPos.xyz());
            mvUpdateSceneDescriptors(am, am.scenes[scene].asset, light, dlight);
        }
        ImGui::End();

        ImGui::Begin("Scene");
        ImGui::Checkbox("Diffuse Mapping", (bool*)&am.scenes[scene].asset.data.doDiffuse);
        ImGui::Checkbox("Normal Mapping", (bool*)&am.scenes[scene].asset.data.doNormal);
        ImGui::Checkbox("Specular Mapping", (bool*)&am.scenes[scene].asset.data.doSpecular);
        ImGui::End();

        mvMat4 viewMatrix = mvCreateFPSView(camera);
        mvMat4 projMatrix = mvCreateLookAtProjection(camera);

        mvBind(am, light, viewMatrix);
        mvBind(am, dlight, viewMatrix);

        Renderer::mvRenderMesh(am, *light.mesh, lightTransform, viewMatrix, projMatrix);
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