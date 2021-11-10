#pragma once

#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "mvTypes.h"

// forward declarations
struct mvScene;
struct mvAssetManager;

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

struct mvPipelineLayout
{
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
};

struct mvPipelineSpec
{
	VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	b8                  backfaceCulling = true;
	b8                  depthTest       = true;
	b8                  depthWrite      = true;
	b8                  wireFrame       = false;
	std::string         vertexShader;
	std::string         pixelShader;
	f32                 width = 0.0f;  // viewport
	f32                 height = 0.0f; // viewport
	VkRenderPass        renderPass = VK_NULL_HANDLE;
};

struct mvPipeline
{
	mvShader       vertexShader;
	mvShader       fragShader;
	mvVertexLayout layout;
	mvAssetID      pipelineLayout;
	VkPipeline     pipeline = VK_NULL_HANDLE;
};


mvPipeline       mvCreatePipeline(mvAssetManager& assetManager, mvPipelineSpec& spec);
mvPipelineLayout mvCreatePipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
