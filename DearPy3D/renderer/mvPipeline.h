#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "mvTypes.h"

// forward declarations
enum mvVertexElementType;
struct mvScene;
struct mvAssetManager;
struct mvGraphics;
struct mvShader;
struct mvVertexLayout;
struct mvPipelineSpec;
struct mvPipeline;
struct mvPipelineManager;


mvVertexLayout    create_vertex_layout(std::vector<mvVertexElementType> elements);
mvPipeline        create_pipeline(mvGraphics& graphics, mvPipelineSpec& spec);
mvPipelineManager create_pipeline_manager();
void              cleanup_pipeline_manager(mvGraphics& graphics, mvPipelineManager& manager);

mvAssetID        register_pipeline(mvPipelineManager& manager, const std::string& tag, mvPipeline pipeline);
mvAssetID        register_pipeline_layout(mvPipelineManager& manager, const std::string& tag, VkPipelineLayout layout);
VkPipeline       get_pipeline(mvPipelineManager& manager, const std::string& tag);
VkPipelineLayout get_pipeline_layout(mvPipelineManager& manager, const std::string& tag);
mvAssetID        reset_pipeline(mvGraphics& graphics, mvPipelineManager& manager, const std::string& tag, mvPipeline& pipeline);

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

struct mvPipelineManager
{

	std::string* pipelineKeys = nullptr;
	std::string* layoutKeys   = nullptr;

	// pipelines	       	  
	u32                   maxPipelineCount = 50u;
	u32                   pipelineCount = 0u;
	mvPipeline*           pipelines = nullptr;

	// pipeline layouts       	  
	u32                    maxLayoutCount = 50u;
	u32                    layoutCount = 0u;
	VkPipelineLayout*      layouts = nullptr;
};