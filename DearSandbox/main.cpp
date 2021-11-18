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
#include <stdlib.h>

mv_internal const char* sponzaPath = "C:/dev/MarvelAssets/Sponza/";
mv_internal b8 loadSponza = false;

int main() 
{

    CreateContext();
    putenv("VK_LAYER_PATH=..\\..\\Dependencies\\vk_sdk_lite\\Bin");
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

    f32 offscreenWidth = 50.0f;
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

    mvPassSpecification offscreenPassSpec{};
    offscreenPassSpec.name = "secondary_pass";
    offscreenPassSpec.depthBias = 0.0f;
    offscreenPassSpec.slopeDepthBias = 0.0f;
    offscreenPassSpec.width = 2048.0f;
    offscreenPassSpec.height = 2048.0f;
    offscreenPassSpec.colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    offscreenPassSpec.depthFormat = VK_FORMAT_D32_SFLOAT;
    offscreenPassSpec.hasColor = true;
    offscreenPassSpec.hasDepth = true;

    mvPassSpecification shadowPassSpec{};
    shadowPassSpec.name = "shadow_pass";
    shadowPassSpec.depthBias = 50.0f;
    shadowPassSpec.slopeDepthBias = 2.0f;
    shadowPassSpec.width = 2048.0f;
    shadowPassSpec.height = 2048.0f;
    shadowPassSpec.depthFormat = VK_FORMAT_D32_SFLOAT;
    shadowPassSpec.hasColor = false;
    shadowPassSpec.hasDepth = true;

    mvPass offscreenPass = Renderer::mvCreateOffscreenRenderPass(am, offscreenPassSpec);
    offscreenPass.pipelineSpec.backfaceCulling = true;
    offscreenPass.pipelineSpec.depthTest = true;
    offscreenPass.pipelineSpec.depthWrite = true;
    offscreenPass.pipelineSpec.wireFrame = false;
    offscreenPass.pipelineSpec.vertexShader = "vs_shader.vert.spv";
    offscreenPass.pipelineSpec.pixelShader = "ps_shader.frag.spv";
    
    offscreenPass.pipelineSpec.layout = mvCreateVertexLayout(
        {
            mvVertexElementType::Position3D,
            mvVertexElementType::Normal,
            mvVertexElementType::Tangent,
            mvVertexElementType::Bitangent,
            mvVertexElementType::Texture2D
        });

    offscreenPass.specification.pipeline = mvRegisterAsset(&am, "secondary_pass", mvCreatePipeline(am, offscreenPass.pipelineSpec));

    mvPass shadowPass = Renderer::mvCreateDepthOnlyRenderPass(am, shadowPassSpec);
    shadowPass.pipelineSpec.backfaceCulling = false;
    shadowPass.pipelineSpec.depthTest = true;
    shadowPass.pipelineSpec.depthWrite = true;
    shadowPass.pipelineSpec.wireFrame = false;
    shadowPass.pipelineSpec.vertexShader = "vs_shadow.vert.spv";
    shadowPass.pipelineSpec.layout = mvCreateVertexLayout(
        {
            mvVertexElementType::Position3D,
            mvVertexElementType::Normal,
            mvVertexElementType::Tangent,
            mvVertexElementType::Bitangent,
            mvVertexElementType::Texture2D
        });

    shadowPass.specification.pipeline = mvRegisterAsset(&am, "shadow_pass", mvCreatePipeline(am, shadowPass.pipelineSpec));


    mvPassSpecification mainPassSpec{};
    mainPassSpec.mainPass = true;
    mainPassSpec.pipeline = mvGetPipelineAssetID(&am, "main_pass");

    mvPass mainPass{
        mainPassSpec,
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
        mvUpdateSceneDescriptors(am, am.scenes[scene].asset, mvGetTextureAssetID(&am, shadowPass.specification.name + std::to_string(GContext->graphics.currentImageIndex)));
        Renderer::mvUpdateDescriptors(am);
        
        mvShowAssetManager(am);

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
        Renderer::mvBeginPass(am, mvGetCurrentCommandBuffer(), shadowPass);

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
        Renderer::mvBeginPass(am, mvGetCurrentCommandBuffer(), offscreenPass);

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
        Renderer::mvBeginPass(am, mvGetCurrentCommandBuffer(), mainPass);

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