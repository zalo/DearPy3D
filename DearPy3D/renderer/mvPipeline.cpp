#include "mvPipeline.h"
#include <stdexcept>
#include <fstream>
#include "mvContext.h"
#include "mvScene.h"
#include "mvMaterials.h"
#include "mvAssetManager.h"

mv_internal std::vector<char>
ReadFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error("failed to open file!");

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

mv_internal mvShader
mvCreateShader(const std::string& file)
{
    mvShader shader{};
    shader.file = file;

    auto shaderCode = ReadFile(GContext->IO.shaderDirectory + file);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(mvGetLogicalDevice(), &createInfo, nullptr, &shader.shaderModule) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module!");

    return shader;
}

struct mvVertexElement
{
    int                 itemCount = 0;
    size_t              size = 0;
    size_t              offset = 0;
    VkFormat            format = VK_FORMAT_UNDEFINED;
    mvVertexElementType type;
};

mvVertexLayout
mvCreateVertexLayout(std::vector<mvVertexElementType> elements)
{
    std::vector<mvVertexElement> velements;
    size_t stride = 0;
    size_t size = 0;

    for (auto& element : elements)
    {
        mvVertexElement newelement;
        newelement.type = element;
        switch (element)
        {

        case mvVertexElementType::Position2D:
            newelement.format = VK_FORMAT_R32G32_SFLOAT;
            newelement.itemCount = 2;
            newelement.size = sizeof(float) * 2;
            break;

        case mvVertexElementType::Position3D:
            newelement.format = VK_FORMAT_R32G32B32_SFLOAT;
            newelement.itemCount = 3;
            newelement.size = sizeof(float) * 3;
            break;

        case mvVertexElementType::Tangent:
            newelement.format = VK_FORMAT_R32G32B32_SFLOAT;
            newelement.itemCount = 3;
            newelement.size = sizeof(float) * 3;
            break;

        case mvVertexElementType::Bitangent:
            newelement.format = VK_FORMAT_R32G32B32_SFLOAT;
            newelement.itemCount = 3;
            newelement.size = sizeof(float) * 3;
            break;

        case mvVertexElementType::Normal:
            newelement.format = VK_FORMAT_R32G32B32_SFLOAT;
            newelement.itemCount = 3;
            newelement.size = sizeof(float) * 3;
            break;

        case mvVertexElementType::Texture2D:
            newelement.format = VK_FORMAT_R32G32_SFLOAT;
            newelement.itemCount = 2;
            newelement.size = sizeof(float) * 2;
            break;

        case mvVertexElementType::Color:
            newelement.format = VK_FORMAT_R32G32B32_SFLOAT;
            newelement.itemCount = 3;
            newelement.size = sizeof(float) * 3;
            break;

        }

        newelement.offset = stride;
        stride += newelement.size;
        velements.push_back(newelement);
    }

    mvVertexLayout vertexLayout{};

    VkVertexInputBindingDescription description{};
    description.binding = 0;
    description.stride = stride;
    description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertexLayout.bindingDescriptions.push_back(description);

    int index = 0;
    for (const auto& element : velements)
    {
        VkVertexInputAttributeDescription description{};
        description.binding = 0;
        description.location = index++;
        description.format = element.format;
        description.offset = element.offset;

        vertexLayout.attributeDescriptions.push_back(description);
    }

    return vertexLayout;
}

mvPipeline
mvCreatePipeline(mvAssetManager& assetManager, mvPipelineSpec& spec)
{

    mvPipeline pipeline{};

    pipeline.vertexShader = mvCreateShader(spec.vertexShader);
    if(!spec.pixelShader.empty())
        pipeline.fragShader = mvCreateShader(spec.pixelShader);
    pipeline.specification = spec;

    //---------------------------------------------------------------------
    // input assembler stage
    //---------------------------------------------------------------------
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = spec.topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = pipeline.specification.layout.attributeDescriptions;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions = pipeline.specification.layout.bindingDescriptions;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    //---------------------------------------------------------------------
    // vertex shader stage
    //---------------------------------------------------------------------
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = pipeline.vertexShader.shaderModule;
    vertShaderStageInfo.pName = "main";

    //---------------------------------------------------------------------
    // tesselation stage
    //---------------------------------------------------------------------

    //---------------------------------------------------------------------
    // geometry shader stage
    //---------------------------------------------------------------------

    //---------------------------------------------------------------------
    // rasterization stage
    //---------------------------------------------------------------------

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = spec.height;
    viewport.width = spec.width;
    viewport.height = -spec.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent.width = (u32)viewport.width;
    scissor.extent.height = (u32)viewport.y;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = spec.wireFrame ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = spec.backfaceCulling ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    if (!spec.pixelShader.empty())
        rasterizer.depthBiasEnable = VK_FALSE;
    else
        rasterizer.depthBiasEnable = VK_TRUE;

    //---------------------------------------------------------------------
    // fragment shader stage
    //---------------------------------------------------------------------
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    if (!spec.pixelShader.empty())
    { 
        fragShaderStageInfo.module = pipeline.fragShader.shaderModule;
    }
    fragShaderStageInfo.pName = "main";

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = spec.depthTest ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = spec.depthWrite ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    //---------------------------------------------------------------------
    // color blending stage
    //---------------------------------------------------------------------
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    if (!spec.pixelShader.empty())
        colorBlending.attachmentCount = 1;
    else
        colorBlending.attachmentCount = 0;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.5f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    //---------------------------------------------------------------------
    // Create Pipeline
    //---------------------------------------------------------------------
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.push_back(vertShaderStageInfo);
    if (!spec.pixelShader.empty())
        shaderStages.push_back(fragShaderStageInfo);

    VkDynamicState dynamicStateEnables[3] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 3;
    dynamicState.pDynamicStates = dynamicStateEnables;
    
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    if (!spec.pixelShader.empty())
        pipelineInfo.layout = mvGetRawPipelineLayoutAsset(&assetManager, "main_pass");
    else
        pipelineInfo.layout = mvGetRawPipelineLayoutAsset(&assetManager, "shadow_pass");
    pipelineInfo.renderPass = spec.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(mvGetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.pipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphics pipeline!");

    // no longer need this
    vkDestroyShaderModule(mvGetLogicalDevice(), pipeline.vertexShader.shaderModule, nullptr);
    if (!spec.pixelShader.empty())
        vkDestroyShaderModule(mvGetLogicalDevice(), pipeline.fragShader.shaderModule, nullptr);
    pipeline.vertexShader.shaderModule = VK_NULL_HANDLE;
    pipeline.fragShader.shaderModule = VK_NULL_HANDLE;

    return pipeline;
}

