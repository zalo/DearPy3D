#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "mvTypes.h"

// convenience
VkDevice         mvGetLogicalDevice();
VkCommandBuffer  mvGetCurrentCommandBuffer();
    
// initialization
void mvSetupGraphicsContext();
void mvRecreateSwapChain();
void mvCleanupGraphicsContext();

// resource utilities
u32    mvFindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
size_t mvGetRequiredUniformBufferSize(size_t size);

void            mvPresent();
VkCommandBuffer mvBeginSingleTimeCommands();
void            mvEndSingleTimeCommands(VkCommandBuffer commandBuffer);

struct mvGraphics
{
    VkInstance                     instance;
    VkDescriptorPool               descriptorPool;
    VkPhysicalDevice               physicalDevice = VK_NULL_HANDLE;
    VkDevice                       logicalDevice = VK_NULL_HANDLE;
    VkQueue                        graphicsQueue = VK_NULL_HANDLE;
    VkQueue                        presentQueue = VK_NULL_HANDLE;
    VkCommandPool                  commandPool = VK_NULL_HANDLE;
    u32                            graphicsQueueFamily = 0;
    std::vector<VkCommandBuffer>   commandBuffers;
    VkPhysicalDeviceProperties     deviceProperties;
    VkFormat                       swapChainImageFormat;
    VkSwapchainKHR                 swapChain = VK_NULL_HANDLE;
    std::vector<VkImage>           swapChainImages;
    std::vector<VkImageView>       swapChainImageViews;
    std::vector<VkFramebuffer>     swapChainFramebuffers;
    VkImage                        depthImage = VK_NULL_HANDLE;
    VkDeviceMemory                 depthImageMemory = VK_NULL_HANDLE;
    VkImageView                    depthImageView = VK_NULL_HANDLE;
    std::vector<VkSemaphore>       imageAvailableSemaphores; // syncronize rendering to image when already rendering to image
    std::vector<VkSemaphore>       renderFinishedSemaphores; // syncronize render/present
    std::vector<VkFence>           inFlightFences;
    std::vector<VkFence>           imagesInFlight;
    VkRenderPass                   renderPass = VK_NULL_HANDLE;
    VkExtent2D                     swapChainExtent;
    u32                            minImageCount = 0;
    u32                            currentImageIndex = 0;
    size_t                         currentFrame = 0;
    VkSurfaceKHR                   surface = VK_NULL_HANDLE;
    std::vector<const char*>       validationLayers;
    std::vector<const char*>       deviceExtensions;
    b8                             enableValidationLayers = true;
    VkDebugUtilsMessengerEXT       debugMessenger;
};
