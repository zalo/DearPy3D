#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "mvAllocator.h"
#include "mvDeletionQueue.h"

namespace DearPy3D {

    //-----------------------------------------------------------------------------
    // forward declarations
    //-----------------------------------------------------------------------------
    struct mvGraphicsContext;

    //-----------------------------------------------------------------------------
    // public API
    //-----------------------------------------------------------------------------

    VkInstance    GetVkInstance();
    VkImageView   CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    std::uint32_t FindMemoryType (VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
    size_t        GetRequiredUniformBufferSize(size_t size);
    void          CreateImage    (VkDevice logicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    void CreateVulkanInstance(std::vector<const char*> validationLayers);
    void SetupDebugMessenger();
    void CreateSurface(GLFWwindow* window, VkSurfaceKHR surface);
    void CreatePhysicalDevice(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties& properties, std::vector<const char*> deviceExtensions);
    void CreateLogicalDevice(mvGraphicsContext& context);
    void CreateSwapChain(mvGraphicsContext& context, int width, int height);
    void CreateMainCommandPool(mvGraphicsContext& context);
    void CreateMainDescriptorPool(VkDevice logicalDevice, VkDescriptorPool* descriptorPool);
    void CreateMainRenderPass(VkDevice logicalDevice, VkRenderPass* renderPass, VkFormat format);
    void CreateMainDepthResources(VkDevice logicalDevice, VkImage depthImage, VkImageView depthImageView, VkDeviceMemory depthImageMemory);
    void CreateFrameBuffers(VkDevice logicalDevice, VkRenderPass renderPass, std::vector<VkFramebuffer>& frameBuffers, std::vector<VkImageView>& swapChainImageViews, VkImageView depthImageView);
    void CreateSyncObjects(VkDevice logicalDevice, std::vector<VkSemaphore>& imgAvailSema, std::vector<VkSemaphore>& imgFinishSema, std::vector<VkFence>& inFlightFences, std::vector<VkFence>& imagesInFlight);

    void RecreateSwapChain(int width, int height);
    void CleanupGraphicsContext();

    struct mvGraphicsContext
    {
        mvDeletionQueue                deletionQueue;
        const int                      max_frames_in_flight = 2;

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
        VkSwapchainKHR                 swapChain;
        std::vector<VkImage>           swapChainImages;
        std::vector<VkImageView>       swapChainImageViews;
        std::vector<VkFramebuffer>     swapChainFramebuffers;
        VkImage                        depthImage;
        VkDeviceMemory                 depthImageMemory;
        VkImageView                    depthImageView;
        std::vector<VkSemaphore>       imageAvailableSemaphores;
        std::vector<VkSemaphore>       renderFinishedSemaphores;
        std::vector<VkFence>           inFlightFences;
        std::vector<VkFence>           imagesInFlight;
        VkRenderPass                   renderPass;
        VkExtent2D                     swapChainExtent;
        uint32_t                       minImageCount = 0;
        uint32_t                       currentImageIndex = 0;
        size_t                         currentFrame = 0;
        VkSurfaceKHR                   surface;

        // maybe can remove
        std::vector<const char*> validationLayers;
        std::vector<const char*> deviceExtensions = ;
        bool enableValidationLayers = true;
        VkDebugUtilsMessengerEXT debugMessenger;
    };

}