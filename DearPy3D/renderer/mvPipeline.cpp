#include "mvPipeline.h"
#include <stdexcept>
#include <fstream>
#include "mvGraphics.h"
#include "mvMaterials.h"

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
mvCreateShader(mvGraphics& graphics, const std::string& file)
{
    mvShader shader{};
    shader.file = file;
    shader.shaderModule = VK_NULL_HANDLE;

    auto shaderCode = ReadFile(graphics.shaderDirectory + file);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(graphics.logicalDevice, &createInfo, nullptr, &shader.shaderModule) != VK_SUCCESS)
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
DearPy3D::create_vertex_layout(std::vector<mvVertexElementType> elements)
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

        case MV_POS_2D:
            newelement.format = VK_FORMAT_R32G32_SFLOAT;
            newelement.itemCount = 2;
            newelement.size = sizeof(float) * 2;
            break;

        case MV_POS_3D:
            newelement.format = VK_FORMAT_R32G32B32_SFLOAT;
            newelement.itemCount = 3;
            newelement.size = sizeof(float) * 3;
            break;

        case MV_TAN_3D:
            newelement.format = VK_FORMAT_R32G32B32_SFLOAT;
            newelement.itemCount = 3;
            newelement.size = sizeof(float) * 3;
            break;

        case MV_BITAN_3D:
            newelement.format = VK_FORMAT_R32G32B32_SFLOAT;
            newelement.itemCount = 3;
            newelement.size = sizeof(float) * 3;
            break;

        case MV_NORM_3D:
            newelement.format = VK_FORMAT_R32G32B32_SFLOAT;
            newelement.itemCount = 3;
            newelement.size = sizeof(float) * 3;
            break;

        case MV_TEXCOORD_3D:
            newelement.format = VK_FORMAT_R32G32_SFLOAT;
            newelement.itemCount = 2;
            newelement.size = sizeof(float) * 2;
            break;

        case MV_COLOR_3D:
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
DearPy3D::create_pipeline(mvGraphics& graphics, mvPipelineSpec& spec)
{

    mvPipeline pipeline;
    pipeline.pipeline = VK_NULL_HANDLE;

    if(!spec.vertexShader.empty())
        pipeline.vertexShader = mvCreateShader(graphics, spec.vertexShader);
    if(!spec.pixelShader.empty())
        pipeline.fragShader = mvCreateShader(graphics, spec.pixelShader);
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
    colorBlendAttachment.blendEnable = spec.blendEnabled ? VK_TRUE : VK_FALSE;
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
    colorBlending.blendConstants[1] = 0.0f;
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
    if (!spec.vertexShader.empty())
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
    pipelineInfo.layout = spec.pipelineLayout;
    pipelineInfo.renderPass = spec.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencil;

    MV_VULKAN(vkCreateGraphicsPipelines(graphics.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.pipeline));

    // no longer need this
    if (!spec.vertexShader.empty())
        vkDestroyShaderModule(graphics.logicalDevice, pipeline.vertexShader.shaderModule, nullptr);
    if (!spec.pixelShader.empty())
        vkDestroyShaderModule(graphics.logicalDevice, pipeline.fragShader.shaderModule, nullptr);
    pipeline.vertexShader.shaderModule = VK_NULL_HANDLE;
    pipeline.fragShader.shaderModule = VK_NULL_HANDLE;

    return pipeline;
}

mvPipelineManager
DearPy3D::create_pipeline_manager()
{
    mvPipelineManager pipelineManager{};
    pipelineManager.pipelineKeys = nullptr;
    pipelineManager.layoutKeys = nullptr;       	  
    pipelineManager.maxPipelineCount = 50u;
    pipelineManager.pipelineCount = 0u;
    pipelineManager.pipelines = nullptr;    	  
    pipelineManager.maxLayoutCount = 50u;
    pipelineManager.layoutCount = 0u;
    pipelineManager.layouts = nullptr;

    pipelineManager.pipelines = new mvPipeline[pipelineManager.maxPipelineCount];
    pipelineManager.pipelineKeys = new std::string[pipelineManager.maxPipelineCount];
    pipelineManager.layouts = new VkPipelineLayout[pipelineManager.maxLayoutCount];
    pipelineManager.layoutKeys = new std::string[pipelineManager.maxLayoutCount];

    for (u32 i = 0; i < pipelineManager.maxPipelineCount; i++)
    {
        pipelineManager.pipelineKeys[i] = "";
    }

    for (u32 i = 0; i < pipelineManager.maxLayoutCount; i++)
    {
        pipelineManager.layoutKeys[i] = "";
        pipelineManager.layouts[i] = VK_NULL_HANDLE;
    }

    return pipelineManager;
}

void
DearPy3D::cleanup_pipeline_manager(mvGraphics& graphics, mvPipelineManager& manager)
{

    // pipeline layouts
    for (int i = 0; i < manager.layoutCount; i++)
    {
        vkDestroyPipelineLayout(graphics.logicalDevice, manager.layouts[i], nullptr);
    }

    // cleanup pipelines
    for (int i = 0; i < manager.pipelineCount; i++)
    {
        vkDestroyPipeline(graphics.logicalDevice, manager.pipelines[i].pipeline, nullptr);
    }

    manager.pipelineCount = 0u;
    manager.layoutCount = 0u;

    delete[] manager.pipelines;
    delete[] manager.pipelineKeys;
    delete[] manager.layouts;
    delete[] manager.layoutKeys;
}

mvAssetID
DearPy3D::register_pipeline(mvPipelineManager& manager, const std::string& tag, mvPipeline pipeline)
{
    assert(manager.pipelineCount <= manager.maxPipelineCount);
    manager.pipelines[manager.pipelineCount] = pipeline;
    manager.pipelineKeys[manager.pipelineCount] = tag;
    manager.pipelineCount++;
    return manager.pipelineCount - 1;
}

mvAssetID
DearPy3D::register_pipeline_layout(mvPipelineManager& manager, const std::string& tag, VkPipelineLayout layout)
{
    assert(manager.layoutCount <= manager.maxLayoutCount);
    manager.layouts[manager.layoutCount] = layout;
    manager.layoutKeys[manager.layoutCount] = tag;
    manager.layoutCount++;
    return manager.layoutCount - 1;
}

VkPipeline
DearPy3D::get_pipeline(mvPipelineManager& manager, const std::string& tag)
{
    for (int i = 0; i < manager.pipelineCount; i++)
    {
        if (manager.pipelineKeys[i] == tag)
            return manager.pipelines[i].pipeline;
    }
    return VK_NULL_HANDLE;
}

VkPipelineLayout
DearPy3D::get_pipeline_layout(mvPipelineManager& manager, const std::string& tag)
{
    for (int i = 0; i < manager.layoutCount; i++)
    {
        if (manager.layoutKeys[i] == tag)
            return manager.layouts[i];
    }
    return VK_NULL_HANDLE;
}

mvAssetID
DearPy3D::reset_pipeline(mvGraphics& graphics, mvPipelineManager& manager, const std::string& tag, mvPipeline& pipeline)
{

    for (int i = 0; i < manager.pipelineCount; i++)
    {
        if (manager.pipelineKeys[i] == tag)
        {
            vkDestroyPipeline(graphics.logicalDevice, manager.pipelines[i].pipeline, nullptr);
            manager.pipelines[i] = pipeline;
            return i;
        }
    }

    return register_pipeline(manager, tag, pipeline);
}