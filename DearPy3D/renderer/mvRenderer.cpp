#include "mvRenderer.h"
#include <stdexcept>
#include <array>
#include "mvContext.h"
#include "mvGraphicsContext.h"
#include "mvDrawable.h"
#include "mvMaterial.h"
#include "mvCamera.h"

namespace DearPy3D {

	void mvRenderer::beginFrame()
	{
		mvBeginFrame();
	}

	void mvRenderer::endFrame()
	{
		mvEndFrame();
		mvPresent();
	}

	void mvRenderer::beginPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("failed to begin recording command buffer!");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = GContext->graphics.swapChainFramebuffers[GContext->graphics.currentImageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = GContext->graphics.swapChainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void mvRenderer::endPass(VkCommandBuffer commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to record command buffer!");
	}

	void mvRenderer::renderDrawable(mvDrawable& drawable, mvPipeline& pipeline, uint32_t index)
	{
		drawable.bind();
		pipeline.bind(index);

		_transforms.model = drawable.getTransform();
		_transforms.modelView = _camera * _transforms.model;
		_transforms.modelViewProjection = _projection * _transforms.modelView;

		vkCmdPushConstants(
			GContext->graphics.commandBuffers[GContext->graphics.currentImageIndex],
			pipeline.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvRenderer::Transforms),&_transforms);

		mvDraw(drawable.getVertexCount());
	}

	void mvRenderer::setCamera(mvCamera& camera)
	{
		_camera = camera.getMatrix();
		_projection = camera.getProjection();
	}

}