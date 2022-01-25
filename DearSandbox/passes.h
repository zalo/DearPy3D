#pragma once

mvPass 
create_main_pass(mvGraphics& graphics, mvAssetManager& am)
{
    mvPassSpecification mainPassSpec{};
    mainPassSpec.mainPass = true;

    mvPass mainPass{
        mainPassSpec,
        graphics.renderPass,
        graphics.swapChainExtent,
        graphics.swapChainFramebuffers
    };

    mainPass.viewport.x = 0.0f;
    mainPass.viewport.y = graphics.swapChainExtent.height;
    mainPass.viewport.width = graphics.swapChainExtent.width;
    mainPass.viewport.height = -(int)graphics.swapChainExtent.height;
    mainPass.viewport.minDepth = 0.0f;
    mainPass.viewport.maxDepth = 1.0f;
    mainPass.extent.width = (u32)graphics.swapChainExtent.width;
    mainPass.extent.height = (u32)mainPass.viewport.y;

    return mainPass;
}

mvPass
create_primary_pass(mvGraphics& graphics, mvAssetManager& am, mvDescriptorManager& dsManager, mvPipelineManager& pmManager, f32 width, f32 height)
{
    mvPassSpecification offscreenPassSpec{};
    offscreenPassSpec.name = "primary_pass";
    offscreenPassSpec.depthBias = 0.0f;
    offscreenPassSpec.slopeDepthBias = 0.0f;
    offscreenPassSpec.width = width;
    offscreenPassSpec.height = height;
    offscreenPassSpec.colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    offscreenPassSpec.depthFormat = VK_FORMAT_D32_SFLOAT;
    offscreenPassSpec.hasColor = true;
    offscreenPassSpec.hasDepth = true;

    mvPass offscreenPass = Renderer::create_primary_pass(graphics, am, offscreenPassSpec);
    offscreenPass.pipelineSpec.backfaceCulling = true;
    offscreenPass.pipelineSpec.depthTest = true;
    offscreenPass.pipelineSpec.depthWrite = true;
    offscreenPass.pipelineSpec.wireFrame = false;
    offscreenPass.pipelineSpec.vertexShader = "phong.vert.spv";
    offscreenPass.pipelineSpec.pixelShader = "phong.frag.spv";
    offscreenPass.pipelineSpec.pipelineLayout = get_pipeline_layout(pmManager, "primary_pass");

    offscreenPass.pipelineSpec.layout = create_vertex_layout(
        {
            mvVertexElementType::Position3D,
            mvVertexElementType::Normal,
            mvVertexElementType::Tangent,
            mvVertexElementType::Bitangent,
            mvVertexElementType::Texture2D
        });

    offscreenPass.specification.pipeline = reset_pipeline(graphics, pmManager, "primary_pass", create_pipeline(graphics, offscreenPass.pipelineSpec));

    {
        mvPipelineSpec pipelineSpec{};
        pipelineSpec.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineSpec.backfaceCulling = false;
        pipelineSpec.depthTest = true;
        pipelineSpec.depthWrite = false;
        pipelineSpec.wireFrame = false;
        pipelineSpec.vertexShader = "skybox.vert.spv";
        pipelineSpec.pixelShader = "skybox.frag.spv";
        pipelineSpec.width = (float)width;
        pipelineSpec.height = (float)height;
        pipelineSpec.renderPass = offscreenPass.renderPass;
        pipelineSpec.pipelineLayout = get_pipeline_layout(pmManager, "skybox_pass");
        pipelineSpec.layout = create_vertex_layout({mvVertexElementType::Position3D});

        reset_pipeline(graphics, pmManager, "skybox_pass", create_pipeline(graphics, pipelineSpec));
    }

    return offscreenPass;
}

mvPass 
create_offscreen_pass(mvGraphics& graphics, mvAssetManager& am, mvDescriptorManager& dsManager, mvPipelineManager& pmManager)
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

    mvPass offscreenPass = Renderer::create_offscreen_pass(graphics, am, offscreenPassSpec);
    offscreenPass.pipelineSpec.backfaceCulling = true;
    offscreenPass.pipelineSpec.depthTest = true;
    offscreenPass.pipelineSpec.depthWrite = true;
    offscreenPass.pipelineSpec.wireFrame = false;
    offscreenPass.pipelineSpec.vertexShader = "phong.vert.spv";
    offscreenPass.pipelineSpec.pixelShader = "phong.frag.spv";
    offscreenPass.pipelineSpec.pipelineLayout = get_pipeline_layout(pmManager, "primary_pass");

    offscreenPass.pipelineSpec.layout = create_vertex_layout(
        {
            mvVertexElementType::Position3D,
            mvVertexElementType::Normal,
            mvVertexElementType::Tangent,
            mvVertexElementType::Bitangent,
            mvVertexElementType::Texture2D
        });

    offscreenPass.specification.pipeline = register_pipeline(pmManager, "secondary_pass", create_pipeline(graphics, offscreenPass.pipelineSpec));

    return offscreenPass;
}

mvPass 
create_shadow_pass(mvGraphics& graphics, mvAssetManager& am, mvDescriptorManager& dsManager, mvPipelineManager& pmManager)
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

    mvPass shadowPass = Renderer::create_depth_pass(graphics, am, shadowPassSpec);
    shadowPass.pipelineSpec.backfaceCulling = false;
    shadowPass.pipelineSpec.depthTest = true;
    shadowPass.pipelineSpec.depthWrite = true;
    shadowPass.pipelineSpec.wireFrame = false;
    shadowPass.pipelineSpec.vertexShader = "shadow.vert.spv";
    shadowPass.pipelineSpec.pipelineLayout = get_pipeline_layout(pmManager, "shadow_pass");
    shadowPass.pipelineSpec.layout = create_vertex_layout(
        {
            mvVertexElementType::Position3D,
            mvVertexElementType::Normal,
            mvVertexElementType::Tangent,
            mvVertexElementType::Bitangent,
            mvVertexElementType::Texture2D
        });

    shadowPass.specification.pipeline = register_pipeline(pmManager, "shadow_pass", create_pipeline(graphics, shadowPass.pipelineSpec));

    return shadowPass;
}

mvPass
create_omnishadow_pass(mvGraphics& graphics, mvAssetManager& am, mvDescriptorManager& dsManager, mvPipelineManager& pmManager)
{
    mvPassSpecification offscreenPassSpec{};
    offscreenPassSpec.name = "omnishadow_pass";
    offscreenPassSpec.depthBias = 50.0f;
    offscreenPassSpec.slopeDepthBias = 2.0f;
    offscreenPassSpec.width = 1024.0f;
    offscreenPassSpec.height = 1024.0f;
    offscreenPassSpec.colorFormat = VK_FORMAT_R32_SFLOAT;
    offscreenPassSpec.depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    offscreenPassSpec.hasColor = true;
    offscreenPassSpec.hasDepth = true;

    mvPass offscreenPass = Renderer::create_omni_shadow_pass(graphics, am, offscreenPassSpec);
    offscreenPass.pipelineSpec.backfaceCulling = false;
    offscreenPass.pipelineSpec.depthTest = true;
    offscreenPass.pipelineSpec.depthWrite = true;
    offscreenPass.pipelineSpec.wireFrame = false;
    offscreenPass.pipelineSpec.blendEnabled = false;
    offscreenPass.pipelineSpec.vertexShader = "omnishadow.vert.spv";
    offscreenPass.pipelineSpec.pixelShader = "omnishadow.frag.spv";
    offscreenPass.pipelineSpec.pipelineLayout = get_pipeline_layout(pmManager, "omnishadow_pass");

    offscreenPass.pipelineSpec.layout = create_vertex_layout(
        {
            mvVertexElementType::Position3D,
            mvVertexElementType::Normal,
            mvVertexElementType::Tangent,
            mvVertexElementType::Bitangent,
            mvVertexElementType::Texture2D
        });

    offscreenPass.specification.pipeline = register_pipeline(pmManager, "omnishadow_pass", create_pipeline(graphics, offscreenPass.pipelineSpec));

    return offscreenPass;
}