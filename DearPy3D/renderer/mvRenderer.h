#pragma once

#include "mvContext.h"
#include "mvMesh.h"

struct mvAssetManager;

struct mvPass
{
	VkRenderPass               renderPass;
	VkExtent2D                 extent;
	std::vector<VkFramebuffer> frameBuffers;
	VkViewport                 viewport;
};

namespace Renderer
{
	void mvBeginFrame();
	void mvEndFrame();
	void mvBeginPass(VkCommandBuffer commandBuffer, mvPass& pass, b8 clear = true);
	void mvEndPass(VkCommandBuffer commandBuffer);

	void mvRenderMesh(mvAssetManager& am, mvMesh& mesh, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection);
	void mvRenderScene(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj);

	void mvUpdateDescriptors(mvAssetManager& am);
}