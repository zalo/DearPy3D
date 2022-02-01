#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "mvDearPy3D.h"

namespace DearPy3D
{
	mvVertexLayout    create_vertex_layout(std::vector<mvVertexElementType> elements);
	mvPipeline        create_pipeline(mvGraphics& graphics, mvPipelineSpec& spec);
	mvPipelineManager create_pipeline_manager();
	void              cleanup_pipeline_manager(mvGraphics& graphics, mvPipelineManager& manager);

	mvAssetID        register_pipeline(mvPipelineManager& manager, const std::string& tag, mvPipeline pipeline);
	mvAssetID        register_pipeline_layout(mvPipelineManager& manager, const std::string& tag, VkPipelineLayout layout);
	VkPipeline       get_pipeline(mvPipelineManager& manager, const std::string& tag);
	VkPipelineLayout get_pipeline_layout(mvPipelineManager& manager, const std::string& tag);
	mvAssetID        reset_pipeline(mvGraphics& graphics, mvPipelineManager& manager, const std::string& tag, mvPipeline& pipeline);
}

enum mvVertexElementType_
{
	MV_POS_2D = 0,
	MV_POS_3D,
	MV_TEXCOORD_3D,
	MV_NORM_3D,
	MV_TAN_3D,
	MV_BITAN_3D,
	MV_COLOR_3D
};

struct mvVertexLayout
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	std::vector<VkVertexInputBindingDescription>   bindingDescriptions;
};

struct mvShader
{
	std::string    file;
	VkShaderModule shaderModule;
};

struct mvPipelineSpec
{
	VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	bool                backfaceCulling;
	bool                depthTest;
	bool                depthWrite;
	bool                wireFrame;
	bool                blendEnabled;
	std::string         vertexShader;
	std::string         pixelShader;
	float               width;  // viewport
	float               height; // viewport
	VkRenderPass        renderPass;
	VkPipelineLayout    pipelineLayout;
	mvVertexLayout      layout;
};

struct mvPipeline
{
	mvPipelineSpec   specification;
	mvShader         vertexShader;
	mvShader         fragShader;
	VkPipelineLayout pipelineLayout;
	VkPipeline       pipeline;
};

struct mvPipelineManager
{

	std::string*      pipelineKeys;
	std::string*      layoutKeys;       	  
	unsigned          maxPipelineCount;
	unsigned          pipelineCount;
	unsigned          maxLayoutCount;
	unsigned          layoutCount;
	mvPipeline*       pipelines;
	VkPipelineLayout* layouts;
};