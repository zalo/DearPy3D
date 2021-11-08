#pragma once

#include <memory>
#include <vulkan/vulkan.h>
#include "mvShader.h"
#include "mvVertexLayout.h"
#include "mvTypes.h"

struct mvScene;
struct mvMaterial;
struct mvAssetManager;

struct mvPipelineLayout
{
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
};

struct mvPipeline
{
	mvShader       vertexShader;
	mvShader       fragShader;
	mvVertexLayout layout;
	mvAssetID      pipelineLayout;
	VkPipeline     pipeline = VK_NULL_HANDLE;
};


mvPipeline       mvCreatePipeline(mvAssetManager& assetManager, mvMaterial& material);
mvPipelineLayout mvCreatePipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
