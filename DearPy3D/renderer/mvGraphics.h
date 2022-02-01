#pragma once

#include "mvDearPy3D.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

// forward declarations
struct sWindow;

namespace DearPy3D
{
    // initialization
    mvGraphics*     setup_graphics_context  (sWindow& viewport, std::vector<const char*> validationLayers);
    void            recreate_swapchain      (mvGraphics& graphics, sWindow& viewport);
    void            cleanup_graphics_context(mvGraphics& graphics);
    void            present                 (mvGraphics& graphics);
    VkCommandBuffer begin_command_buffer    (mvGraphics& graphics);                                // single use command buffer (submit with next command)
    void            submit_command_buffer   (mvGraphics& graphics, VkCommandBuffer commandBuffer); // frees command buffer

    // convenience
    VkCommandBuffer get_current_command_buffer(mvGraphics& graphics);

    // resource utilities
    u32    find_memory_type(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
    size_t get_required_uniform_buffer_size(mvGraphics& graphics, size_t size); // Calculate required alignment based on minimum device offset alignment
}

struct mvGraphics
{
    std::string                    shaderDirectory;
    VkInstance                     instance;
    VkDescriptorPool               descriptorPool;
    VkPhysicalDevice               physicalDevice;
    VkDevice                       logicalDevice;
    VkQueue                        graphicsQueue;
    VkQueue                        presentQueue;
    VkCommandPool                  commandPool;
    unsigned                       graphicsQueueFamily;
    std::vector<VkCommandBuffer>   commandBuffers;
    VkPhysicalDeviceProperties     deviceProperties;
    VkFormat                       swapChainImageFormat;
    VkSwapchainKHR                 swapChain;
    std::vector<VkImage>           swapChainImages;
    std::vector<VkImageView>       swapChainImageViews;
    std::vector<VkFramebuffer>     swapChainFramebuffers;
    VkImage                        depthImage;
    VkDeviceMemory                 depthImageMemory;
    VkImageView                    depthImageView;
    std::vector<VkSemaphore>       imageAvailableSemaphores; // syncronize rendering to image when already rendering to image
    std::vector<VkSemaphore>       renderFinishedSemaphores; // syncronize render/present
    std::vector<VkFence>           inFlightFences;
    std::vector<VkFence>           imagesInFlight;
    VkRenderPass                   renderPass;
    VkExtent2D                     swapChainExtent;
    unsigned                       minImageCount;
    unsigned                       currentImageIndex;
    size_t                         currentFrame;
    VkSurfaceKHR                   surface;
    bool                           enableValidationLayers;
    VkDebugUtilsMessengerEXT       debugMessenger;
};
