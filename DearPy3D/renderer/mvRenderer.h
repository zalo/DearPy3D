#pragma once

#include "mvContext.h"
#include "mvMesh.h"

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
	void mvRenderMesh(const mvMesh& drawable, mvMaterial& material, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection);
}