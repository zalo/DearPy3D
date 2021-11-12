#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <vk_mem_alloc.h>
#include "mvTypes.h"

//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
struct mvGraphicsContext;

//-----------------------------------------------------------------------------
// public API
//-----------------------------------------------------------------------------

// convenience
VkDevice         mvGetLogicalDevice();
VkCommandBuffer  mvGetCurrentCommandBuffer();
    
// initialization
void mvSetupGraphicsContext();
void mvRecreateSwapChain();
void mvCleanupGraphicsContext();
void mvCreateRenderPass(VkFormat format, VkRenderPass* renderPass);
void mvCreateFrameBuffer(VkRenderPass renderPass, VkFramebuffer& frameBuffer, u32 width, u32 height, std::vector<VkImageView> imageViews);

// allocator
VmaAllocation mvAllocateBuffer(VkBufferCreateInfo bufferCreateInfo, VmaMemoryUsage usage, VkBuffer& outBuffer);
VmaAllocation mvAllocateImage(VkImageCreateInfo imageCreateInfo, VmaMemoryUsage usage, VkImage& outImage);
void          mvFree(VmaAllocation allocation);
void          mvDestroyImage(VkImage image, VmaAllocation allocation);
void          mvDestroyBuffer(VkBuffer buffer, VmaAllocation allocation);
void*         mvMapMemory(VmaAllocation allocation);
void          mvUnmapMemory(VmaAllocation allocation);

// resource utilities
void          mvCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
void          mvCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void          mvTransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
void          mvCopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
VkImageView   mvCreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
std::uint32_t mvFindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
size_t        mvGetRequiredUniformBufferSize(size_t size);
void          mvCreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

void            mvRecordImGui(VkCommandBuffer commandBuffer);
void            mvPresent();
VkCommandBuffer mvBeginSingleTimeCommands();
void            mvEndSingleTimeCommands(VkCommandBuffer commandBuffer);

struct mvGraphics
{

    // internal
    VkInstance                     instance;
    VkDescriptorPool               descriptorPool;
    VkPhysicalDevice               physicalDevice = VK_NULL_HANDLE;
    VkDevice                       logicalDevice = VK_NULL_HANDLE;
    VkQueue                        graphicsQueue = VK_NULL_HANDLE;
    VkQueue                        presentQueue = VK_NULL_HANDLE;
    VkCommandPool                  commandPool = VK_NULL_HANDLE;
    uint32_t                       graphicsQueueFamily = 0;
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
    uint32_t                       minImageCount = 0;
    uint32_t                       currentImageIndex = 0;
    size_t                         currentFrame = 0;
    VkSurfaceKHR                   surface = VK_NULL_HANDLE;
    VmaAllocator                   allocator;

    // maybe can remove
    std::vector<const char*> validationLayers;
    std::vector<const char*> deviceExtensions;
    bool enableValidationLayers = true;
    VkDebugUtilsMessengerEXT debugMessenger;
};
