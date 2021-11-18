#pragma once

#include "mvContext.h"
#include "mvMesh.h"

struct mvAssetManager;

struct mvPassSpecification
{
	std::string name;
	mvAssetID   pipeline           = MV_INVALID_ASSET_ID;
	f32         clearColorValue[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	f32         clearDepthValue    = 1.0f;
	f32         depthBias          = 0.0f;
	f32         slopeDepthBias     = 0.0f;
	f32         width              = 0.0f;
	f32         height             = 0.0f;
	VkFormat    colorFormat        = VK_FORMAT_R8G8B8A8_UNORM;
	VkFormat    depthFormat        = VK_FORMAT_D32_SFLOAT;
	b8          hasColor           = true;
	b8          hasDepth           = true;
	b8          mainPass           = false;
};

struct mvPass
{
	mvPassSpecification        specification;
	VkRenderPass               renderPass;
	VkExtent2D                 extent;
	std::vector<VkFramebuffer> frameBuffers;
	std::vector<mvTexture>     colorTextures;
	std::vector<mvTexture>     depthTextures;
	VkViewport                 viewport;
	mvPipelineSpec             pipelineSpec;
};

namespace Renderer
{
	void mvBeginFrame();
	void mvEndFrame();
	void mvBeginPass(mvAssetManager& am, VkCommandBuffer commandBuffer, mvPass& pass);
	void mvEndPass(VkCommandBuffer commandBuffer);

	mvPass mvCreateOffscreenRenderPass(mvAssetManager& am, mvPassSpecification specification);
	mvPass mvCreateDepthOnlyRenderPass(mvAssetManager& am, mvPassSpecification specification);

	void mvRenderMesh(mvAssetManager& am, mvMesh& mesh, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection);
	void mvRenderScene(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj);

	void mvRenderMeshShadow(mvAssetManager& am, mvMesh& mesh, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection);
	void mvRenderSceneShadow(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj);

	void mvUpdateDescriptors(mvAssetManager& am);
}