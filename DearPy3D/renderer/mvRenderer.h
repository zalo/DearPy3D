#pragma once

#include "mvContext.h"
#include "mvMesh.h"

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
		void mvRenderMesh(const mvMesh& drawable, mvPipeline& pipeline, mvTransforms transforms, glm::mat4 camera, glm::mat4 projection);
	}
}