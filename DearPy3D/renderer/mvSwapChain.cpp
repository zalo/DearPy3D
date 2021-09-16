#include "mvSwapChain.h"
#include <stdexcept>
#include "mvGraphics.h"

namespace DearPy3D {

    void mvSwapChain::resize(float width, float height)
    {

        init(width, height);
        mvGraphics::GetContext().getLogicalDevice().createCommandPool(mvGraphics::GetContext().getPhysicalDevice());
        createRenderPass();
        createDepthResources();
        createFrameBuffers();
    }

    void mvSwapChain::beginFrame()
    {
        vkWaitForFences(mvGraphics::GetContext().getLogicalDevice(), 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

        vkAcquireNextImageKHR(mvGraphics::GetContext().getLogicalDevice(), _swapChain, UINT64_MAX, _imageAvailableSemaphores[_currentFrame],
            VK_NULL_HANDLE, &_currentImageIndex);

        if (_imagesInFlight[_currentImageIndex] != VK_NULL_HANDLE)
            vkWaitForFences(mvGraphics::GetContext().getLogicalDevice(), 1, &_imagesInFlight[_currentImageIndex], VK_TRUE, UINT64_MAX);

        _imagesInFlight[_currentImageIndex] = _inFlightFences[_currentFrame];
    }

    void mvSwapChain::present()
    {

        VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[_currentFrame] };

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { _swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &_currentImageIndex;

        vkQueuePresentKHR(mvGraphics::GetContext().getLogicalDevice().getPresentQueue(), &presentInfo);

        _currentFrame = (_currentFrame + 1) % _max_frames_in_flight;
    }

    void mvSwapChain::draw(uint32_t vertexCount)
    {
        vkCmdDrawIndexed(mvGraphics::GetContext().getLogicalDevice().getCommandBuffers()[_currentImageIndex], vertexCount, 1, 0, 0, 0);
    }

    void mvSwapChain::endFrame()
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &mvGraphics::GetContext().getLogicalDevice().getCommandBuffers()[_currentImageIndex];

        VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[_currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(mvGraphics::GetContext().getLogicalDevice(), 1, &_inFlightFences[_currentFrame]);

        if (vkQueueSubmit(mvGraphics::GetContext().getLogicalDevice().getGraphicsQueue(), 1, &submitInfo, _inFlightFences[_currentFrame]) != VK_SUCCESS)
            throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkCommandBuffer mvSwapChain::getCurrentCommandBuffer()
    { 
        return mvGraphics::GetContext().getLogicalDevice().getCommandBuffers()[_currentImageIndex];
    }

    VkRenderPassBeginInfo mvSwapChain::getMainRenderPassInfo()
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = _renderPass;
        renderPassInfo.framebuffer = _swapChainFramebuffers[_currentImageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = _swapChainExtent;

        return renderPassInfo;
    }

}