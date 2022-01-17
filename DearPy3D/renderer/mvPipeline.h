#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "mvTypes.h"

// forward declarations
struct mvScene;
struct mvAssetManager;
struct mvGraphics;

enum mvVertexElementType
{
	Position2D,
	Position3D,
	Texture2D,
	Normal,
	Tangent,
	Bitangent,
	Color
};

struct mvVertexLayout
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	std::vector<VkVertexInputBindingDescription>   bindingDescriptions;
};

struct mvShader
{
	std::string    file;
	VkShaderModule shaderModule = VK_NULL_HANDLE;
};

struct mvPipelineSpec
{
	VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	b8                  backfaceCulling = true;
	b8                  depthTest       = true;
	b8                  depthWrite      = true;
	b8                  wireFrame       = false;
	b8                  blendEnabled    = true;
	std::string         vertexShader;
	std::string         pixelShader;
	f32                 width = 0.0f;  // viewport
	f32                 height = 0.0f; // viewport
	VkRenderPass        renderPass = VK_NULL_HANDLE;
	VkPipelineLayout    pipelineLayout = VK_NULL_HANDLE;
	mvVertexLayout      layout;
};

struct mvPipeline
{
	mvPipelineSpec   specification;
	mvShader         vertexShader;
	mvShader         fragShader;
	VkPipelineLayout pipelineLayout;
	VkPipeline       pipeline = VK_NULL_HANDLE;
};


mvPipeline     create_pipeline(mvGraphics& graphics, mvAssetManager& assetManager, mvPipelineSpec& spec);
mvVertexLayout create_vertex_layout(std::vector<mvVertexElementType> elements);
