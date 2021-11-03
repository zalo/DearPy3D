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
	void mvRenderMesh(mvAssetManager& am, mvAssetID pipelineLayout, const mvMesh& drawable, mvAssetID material, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection);
}