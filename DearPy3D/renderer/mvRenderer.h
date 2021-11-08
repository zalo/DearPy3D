#pragma once

#include "mvContext.h"
#include "mvMesh.h"

struct mvAssetManager;

namespace Renderer
{
	void mvResize();
	void mvStartRenderer();
	void mvStopRenderer();
	void mvBeginFrame();
	void mvEndFrame();
	void mvPresent();
	void mvBeginPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass);
	void mvEndPass(VkCommandBuffer commandBuffer);

	void mvRenderMesh(mvAssetManager& am, mvMesh& mesh, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection);
	void mvRenderScene(mvAssetManager& am, mvScene& scene, mvMat4 cam, mvMat4 proj);

	void mvPreLoadAssets(mvAssetManager& am);
}