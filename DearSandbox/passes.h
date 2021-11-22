#pragma once

mvPass 
create_main_pass(mvAssetManager& am)
{
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

    return mainPass;
}

mvPass 
create_offscreen_pass(mvAssetManager& am)
{
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

    mvPass offscreenPass = Renderer::mvCreateOffscreenRenderPass(am, offscreenPassSpec);
    offscreenPass.pipelineSpec.backfaceCulling = true;
    offscreenPass.pipelineSpec.depthTest = true;
    offscreenPass.pipelineSpec.depthWrite = true;
    offscreenPass.pipelineSpec.wireFrame = false;
    offscreenPass.pipelineSpec.vertexShader = "phong.vert.spv";
    offscreenPass.pipelineSpec.pixelShader = "phong.frag.spv";
    offscreenPass.pipelineSpec.pipelineLayout = mvGetRawPipelineLayoutAsset(&am, "main_pass");

    offscreenPass.pipelineSpec.layout = mvCreateVertexLayout(
        {
            mvVertexElementType::Position3D,
            mvVertexElementType::Normal,
            mvVertexElementType::Tangent,
            mvVertexElementType::Bitangent,
            mvVertexElementType::Texture2D
        });

    offscreenPass.specification.pipeline = mvRegisterAsset(&am, "secondary_pass", mvCreatePipeline(am, offscreenPass.pipelineSpec));

    return offscreenPass;
}

mvPass 
create_shadow_pass(mvAssetManager& am)
{
    mvPassSpecification shadowPassSpec{};
    shadowPassSpec.name = "shadow_pass";
    shadowPassSpec.depthBias = 50.0f;
    shadowPassSpec.slopeDepthBias = 2.0f;
    shadowPassSpec.width = 2048.0f;
    shadowPassSpec.height = 2048.0f;
    shadowPassSpec.depthFormat = VK_FORMAT_D32_SFLOAT;
    shadowPassSpec.hasColor = false;
    shadowPassSpec.hasDepth = true;

    mvPass shadowPass = Renderer::mvCreateDepthOnlyRenderPass(am, shadowPassSpec);
    shadowPass.pipelineSpec.backfaceCulling = false;
    shadowPass.pipelineSpec.depthTest = true;
    shadowPass.pipelineSpec.depthWrite = true;
    shadowPass.pipelineSpec.wireFrame = false;
    shadowPass.pipelineSpec.vertexShader = "shadow.vert.spv";
    shadowPass.pipelineSpec.pipelineLayout = mvGetRawPipelineLayoutAsset(&am, "shadow_pass");
    shadowPass.pipelineSpec.layout = mvCreateVertexLayout(
        {
            mvVertexElementType::Position3D,
            mvVertexElementType::Normal,
            mvVertexElementType::Tangent,
            mvVertexElementType::Bitangent,
            mvVertexElementType::Texture2D
        });

    shadowPass.specification.pipeline = mvRegisterAsset(&am, "shadow_pass", mvCreatePipeline(am, shadowPass.pipelineSpec));

    return shadowPass;
}