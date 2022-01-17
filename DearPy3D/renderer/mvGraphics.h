#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "mvTypes.h"

// forward declarations
struct mvGraphics;
struct mvViewport;

// initialization
void            setup_graphics_context  (mvGraphics& graphics, mvViewport& viewport);
void            recreate_swapchain      (mvGraphics& graphics);
void            cleanup_graphics_context(mvGraphics& graphics);
void            present                 (mvGraphics& graphics);
VkCommandBuffer begin_command_buffer    (mvGraphics& graphics);                                // single use command buffer (submit with next command)
void            submit_command_buffer   (mvGraphics& graphics, VkCommandBuffer commandBuffer); // frees command buffer

// convenience
VkCommandBuffer get_current_command_buffer(mvGraphics& graphics);

// resource utilities
u32             find_memory_type(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
size_t          get_required_uniform_buffer_size(mvGraphics& graphics, size_t size); // Calculate required alignment based on minimum device offset alignment

struct mvGraphics
{
    VkInstance                     instance;
    VkDescriptorPool               descriptorPool;
    VkPhysicalDevice               physicalDevice = VK_NULL_HANDLE;
    VkDevice                       logicalDevice  = VK_NULL_HANDLE;
    VkQueue                        graphicsQueue  = VK_NULL_HANDLE;
    VkQueue                        presentQueue   = VK_NULL_HANDLE;
    VkCommandPool                  commandPool    = VK_NULL_HANDLE;
    u32                            graphicsQueueFamily = 0;
    std::vector<VkCommandBuffer>   commandBuffers;
    VkPhysicalDeviceProperties     deviceProperties;
    VkFormat                       swapChainImageFormat;
    VkSwapchainKHR                 swapChain = VK_NULL_HANDLE;
    std::vector<VkImage>           swapChainImages;
    std::vector<VkImageView>       swapChainImageViews;
    std::vector<VkFramebuffer>     swapChainFramebuffers;
    VkImage                        depthImage       = VK_NULL_HANDLE;
    VkDeviceMemory                 depthImageMemory = VK_NULL_HANDLE;
    VkImageView                    depthImageView   = VK_NULL_HANDLE;
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
