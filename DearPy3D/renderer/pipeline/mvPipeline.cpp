#include "mvPipeline.h"
#include <stdexcept>
#include "mvGraphics.h"
#include "mvDescriptorSet.h"

namespace DearPy3D {

	void mvPipeline::setVertexLayout(mvVertexLayout layout)
	{
		_layout = layout;
	}

	void mvPipeline::setVertexShader(mvGraphics& graphics, const std::string& file)
	{
        _vertexShader = std::make_unique<mvShader>(graphics, file);
	}

	void mvPipeline::setFragmentShader(mvGraphics& graphics, const std::string& file)
	{
		_fragShader = std::make_unique<mvShader>(graphics, file);
	}

    void mvPipeline::setDescriptorSetLayout(std::shared_ptr<mvDescriptorSetLayout> layout)
    {
        _descriptorSetLayout = layout;
    }

    void mvPipeline::setDescriptorSets(std::vector<std::shared_ptr<mvDescriptorSet>> descriptorSets)
    {
        _descriptorSets = descriptorSets;
    }

    void mvPipeline::bind(mvGraphics& graphics)
    {
        vkCmdBindPipeline(graphics.getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
    }

    void mvPipeline::finish(mvGraphics& graphics)
    {
        vkDestroyPipeline(graphics.getDevice(), _pipeline, nullptr);
        vkDestroyPipelineLayout(graphics.getDevice(), _pipelineLayout, nullptr);
    }

	void mvPipeline::finalize(mvGraphics& graphics)
	{
        //---------------------------------------------------------------------
        // input assembler stage
        //---------------------------------------------------------------------
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        auto attributeDescriptions = _layout.getAttributeDescriptions();
        auto bindingDescriptions = _layout.getBindingDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        //---------------------------------------------------------------------
        // vertex shader stage
        //---------------------------------------------------------------------
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = _vertexShader->getShaderModule();
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
        viewport.y = 0.0f;
        viewport.width = (float)graphics.getSwapChainExtent().width;
        viewport.height = (float)graphics.getSwapChainExtent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = graphics.getSwapChainExtent();

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
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        //---------------------------------------------------------------------
        // fragment shader stage
        //---------------------------------------------------------------------
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = _fragShader->getShaderModule();
        fragShaderStageInfo.pName = "main";

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
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
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.5f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        //---------------------------------------------------------------------
        // Create Pipeline layout
        //---------------------------------------------------------------------

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pSetLayouts = _descriptorSetLayout->getLayout();

        if (vkCreatePipelineLayout(graphics.getDevice(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout!");

        //---------------------------------------------------------------------
        // Create Pipeline
        //---------------------------------------------------------------------

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = _pipelineLayout;
        pipelineInfo.renderPass = graphics.getMainRenderPassInfo().renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState = &depthStencil;

        if (vkCreateGraphicsPipelines(graphics.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline) != VK_SUCCESS)
            throw std::runtime_error("failed to create graphics pipeline!");

        // no longer need this
        vkDestroyShaderModule(graphics.getDevice(), _vertexShader->getShaderModule(), nullptr);
        vkDestroyShaderModule(graphics.getDevice(), _fragShader->getShaderModule(), nullptr);
        _vertexShader = nullptr;
        _fragShader = nullptr;

        graphics.getDeletionQueue().pushDeletor([=, &graphics]() {
            vkDestroyPipeline(graphics.getDevice(), _pipeline, nullptr);
            vkDestroyPipelineLayout(graphics.getDevice(), _pipelineLayout, nullptr);
            });
	}

}