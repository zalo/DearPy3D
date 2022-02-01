#include "mvRenderer.h"
#include <stdexcept>
#include <imgui_impl_win32.h>
#include <imgui_impl_vulkan.h>
#include "mvGraphics.h"

mvRendererContext
DearPy3D::create_renderer_context(mvGraphics& graphics)
{
    mvRendererContext ctx;
    ctx.meshCount = 0u;
    ctx.graphics = &graphics;
    ctx.pipelineManager = create_pipeline_manager();
    ctx.descriptorManager = create_descriptor_manager();
    ctx.materialManager = create_material_manager();
    ctx.textureManager = create_texture_manager();
    return ctx;
}

void
DearPy3D::cleanup_renderer_context(mvRendererContext& ctx)
{
    MV_VULKAN(vkDeviceWaitIdle(ctx.graphics->logicalDevice));
    cleanup_descriptor_manager(*ctx.graphics, ctx.descriptorManager);
    cleanup_pipeline_manager(*ctx.graphics, ctx.pipelineManager);
    cleanup_material_manager(*ctx.graphics, ctx.materialManager);
    cleanup_texture_manager(*ctx.graphics, ctx.textureManager);

    for (int i = 0; i < ctx.meshCount; i++)
    {
        vkDestroyBuffer(ctx.graphics->logicalDevice, ctx.meshes[i].indexBuffer.buffer, nullptr);
        vkDestroyBuffer(ctx.graphics->logicalDevice, ctx.meshes[i].vertexBuffer.buffer, nullptr);
        vkFreeMemory(ctx.graphics->logicalDevice, ctx.meshes[i].indexBuffer.deviceMemory, nullptr);
        vkFreeMemory(ctx.graphics->logicalDevice, ctx.meshes[i].vertexBuffer.deviceMemory, nullptr);

        ctx.meshes[i].indexBuffer.buffer = VK_NULL_HANDLE;
        ctx.meshes[i].indexBuffer.deviceMemory = VK_NULL_HANDLE;
        ctx.meshes[i].indexBuffer.specification.count = 0u;
        ctx.meshes[i].indexBuffer.specification.aligned = false;
        ctx.meshes[i].vertexBuffer.buffer = VK_NULL_HANDLE;
        ctx.meshes[i].vertexBuffer.deviceMemory = VK_NULL_HANDLE;
        ctx.meshes[i].vertexBuffer.specification.count = 0u;
        ctx.meshes[i].vertexBuffer.specification.aligned = false;
    }
    ctx.meshCount = 0u;
    ctx.graphics = nullptr;
}

void
DearPy3D::begin_frame(mvRendererContext& ctx)
{
    mvGraphics& graphics = *ctx.graphics;

    MV_VULKAN(vkWaitForFences(graphics.logicalDevice, 1, &graphics.inFlightFences[graphics.currentFrame], VK_TRUE, UINT64_MAX));

    MV_VULKAN(vkAcquireNextImageKHR(graphics.logicalDevice, graphics.swapChain, UINT64_MAX, graphics.imageAvailableSemaphores[graphics.currentFrame],
        VK_NULL_HANDLE, &graphics.currentImageIndex));

    if (graphics.imagesInFlight[graphics.currentImageIndex] != VK_NULL_HANDLE)
        MV_VULKAN(vkWaitForFences(graphics.logicalDevice, 1, &graphics.imagesInFlight[graphics.currentImageIndex], VK_TRUE, UINT64_MAX));

    // just in case the acquired image is out of order
    graphics.imagesInFlight[graphics.currentImageIndex] = graphics.inFlightFences[graphics.currentFrame];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    MV_VULKAN(vkBeginCommandBuffer(get_current_command_buffer(graphics), &beginInfo));
}

void
DearPy3D::end_frame(mvRendererContext& ctx)
{
    mvGraphics& graphics = *ctx.graphics;

    MV_VULKAN(vkEndCommandBuffer(get_current_command_buffer(graphics)));

    VkSemaphore waitSemaphores[] = { graphics.imageAvailableSemaphores[graphics.currentFrame] };
    VkSemaphore signalSemaphores[] = { graphics.renderFinishedSemaphores[graphics.currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &graphics.commandBuffers[graphics.currentImageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    MV_VULKAN(vkResetFences(graphics.logicalDevice, 1, &graphics.inFlightFences[graphics.currentFrame]));
    MV_VULKAN(vkQueueSubmit(graphics.graphicsQueue, 1, &submitInfo, graphics.inFlightFences[graphics.currentFrame]));
}

void
DearPy3D::begin_pass(mvRendererContext& ctx, VkCommandBuffer commandBuffer, mvPass& pass)
{
    mvGraphics& graphics = *ctx.graphics;

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = pass.renderPass;
    renderPassInfo.framebuffer = pass.frameBuffers[graphics.currentImageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = pass.extent;

    VkClearValue clearValues[2];
    if (pass.specification.hasColor && pass.specification.hasDepth)
    {

        clearValues[0].color = *(VkClearColorValue*)&pass.specification.clearColorValue.x;
        clearValues[1].depthStencil = { pass.specification.clearDepthValue, 0 };
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearValues;
    }
    else if (pass.specification.hasColor)
    {

        clearValues[0].color = *(VkClearColorValue*)&pass.specification.clearColorValue.x;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = clearValues;
    }
    else if (pass.specification.hasDepth)
    {
        clearValues[0].depthStencil = { pass.specification.clearDepthValue, 0 };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = clearValues;
    }

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdSetViewport(commandBuffer, 0, 1, &pass.viewport);

    VkRect2D scissor{};
    scissor.extent = pass.extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdSetDepthBias(
        get_current_command_buffer(graphics),
        pass.specification.depthBias,
        0.0f,
        pass.specification.slopeDepthBias);

    if (pass.specification.pipeline != MV_INVALID_ASSET_ID)
    {
        VkPipeline pipeline = ctx.pipelineManager.pipelines[pass.specification.pipeline].pipeline;
        vkCmdBindPipeline(get_current_command_buffer(graphics), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }
}

void
DearPy3D::end_pass(VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
}

void 
DearPy3D::cleanup_pass(mvGraphics& graphics, mvPass& pass)
{
    vkDeviceWaitIdle(graphics.logicalDevice);
    for (u32 i = 0; i < graphics.swapChainFramebuffers.size(); i++)
    {
        if (pass.specification.hasColor)
        {
            vkDestroySampler(graphics.logicalDevice, pass.colorTextures[i].imageInfo.sampler, nullptr);
            vkDestroyImage(graphics.logicalDevice, pass.colorTextures[i].textureImage, nullptr);
            vkDestroyImageView(graphics.logicalDevice, pass.colorTextures[i].imageInfo.imageView, nullptr);
            vkFreeMemory(graphics.logicalDevice, pass.colorTextures[i].textureImageMemory, nullptr);
        }

        if (pass.specification.hasDepth)
        {
            vkDestroySampler(graphics.logicalDevice, pass.depthTextures[i].imageInfo.sampler, nullptr);
            vkDestroyImage(graphics.logicalDevice, pass.depthTextures[i].textureImage, nullptr);
            vkDestroyImageView(graphics.logicalDevice, pass.depthTextures[i].imageInfo.imageView, nullptr);
            vkFreeMemory(graphics.logicalDevice, pass.depthTextures[i].textureImageMemory, nullptr);
        }

        vkDestroyFramebuffer(graphics.logicalDevice, pass.frameBuffers[i], nullptr);

    }

    vkDestroyRenderPass(graphics.logicalDevice, pass.renderPass, nullptr);

    pass.renderPass = VK_NULL_HANDLE;
}
