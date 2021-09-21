#pragma once

#include "mvContext.h"
#include "mvDrawable.h"

namespace DearPy3D {

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
		void mvRenderDrawable(const mvDrawable& drawable, const mvPipeline& pipeline, uint32_t index, mvTransforms transforms, glm::mat4 camera, glm::mat4 projection);
	}
}