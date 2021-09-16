#include "mvGraphicsContext.h"
#include <stdexcept>
#include <iostream>
#include <set>
#include <optional>
#include <array>
#include "mvContext.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
    VkDebugUtilsMessageTypeFlagsEXT messageType, 
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

namespace DearPy3D {

    //-----------------------------------------------------------------------------
    // forward declarations
    //-----------------------------------------------------------------------------

    struct SwapChainSupportDetails;
    struct QueueFamilyIndices;

    static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    static bool                    CheckValidationLayerSupport(const std::vector<const char*> layers);
    static bool                    IsDeviceSuitable(VkPhysicalDevice device, VkPhysicalDeviceProperties& properties, std::vector<const char*> deviceExtensions);
    static bool                    CheckDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> deviceExtensions);

    //-----------------------------------------------------------------------------
    // internals
    //-----------------------------------------------------------------------------

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        // todo: put in appropriate spot
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, 0, surface, &presentSupport);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    static bool CheckValidationLayerSupport(const std::vector<const char*> layers)
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : layers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    static bool IsDeviceSuitable(VkPhysicalDevice device, VkPhysicalDeviceProperties& properties, std::vector<const char*> deviceExtensions)
    {
        QueueFamilyIndices indices = FindQueueFamilies(device);

        bool extensionsSupported = CheckDeviceExtensionSupport(device, deviceExtensions);

        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            swapChainAdequate = true;
            //SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            //swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        vkGetPhysicalDeviceProperties(device, &properties);

        std::cout << "The GPU has a minimum buffer alignment of " << properties.limits.minUniformBufferOffsetAlignment << std::endl;

        return extensionsSupported && swapChainAdequate && properties.limits.maxPushConstantsSize >= 256;
    }

    static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
    {

        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
                indices.presentFamily = i;
                break;
            }
            i++;
        }

        return indices;
    }

    static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> deviceExtensions)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions)
            requiredExtensions.erase(extension.extensionName);

        return requiredExtensions.empty();
    }

    //-----------------------------------------------------------------------------
    // declarations for public API
    //-----------------------------------------------------------------------------

    VkInstance GetVkInstance()
    {
        return GContext->graphics.instance;
    }

    void CreateVulkanInstance(std::vector<const char*> validationLayers)
    {

        bool enableValidationLayers = !validationLayers.empty();
        GContext->graphics.enableValidationLayers = enableValidationLayers;
        GContext->graphics.validationLayers = validationLayers;

        if (enableValidationLayers && !CheckValidationLayerSupport(validationLayers))
            throw std::runtime_error("validation layers requested, but not available!");

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // get extensions required to load glfw
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (enableValidationLayers)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // Setup debug messenger for vulkan instance
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = GContext->graphics.validationLayers.data();
            createInfo.pNext = VK_NULL_HANDLE;

            debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = debugCallback;
            debugCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = VK_NULL_HANDLE;
        }

        // create the instance
        if (vkCreateInstance(&createInfo, nullptr, &GContext->graphics.instance) != VK_SUCCESS)
            throw std::runtime_error("failed to create instance!");
    }

    void SetupDebugMessenger()
    {

        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pNext = VK_NULL_HANDLE;

        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(GContext->graphics.instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            VkResult result = func(GetVkInstance(), &createInfo, nullptr, &GContext->graphics.debugMessenger);
            if (result != VK_SUCCESS)
                throw std::runtime_error("failed to set up debug messenger!");
        }
        else
            throw std::runtime_error("failed to set up debug messenger!");
    }

    void CreateSurface(GLFWwindow* window, VkSurfaceKHR surface)
    {
        if (glfwCreateWindowSurface(GetVkInstance(), window, nullptr, &surface) != VK_SUCCESS)
            throw std::runtime_error("failed to create window surface!");
    }

    void CreatePhysicalDevice(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties& properties, std::vector<const char*> deviceExtensions)
    {
        GContext->graphics.deviceExtensions = deviceExtensions;
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(GetVkInstance(), &deviceCount, nullptr);

        if (deviceCount == 0)
            throw std::runtime_error("failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(GetVkInstance(), &deviceCount, devices.data());

        for (const auto& device : devices)
        {
            if (IsDeviceSuitable(device, properties, deviceExtensions))
            {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
            throw std::runtime_error("failed to find a suitable GPU!");
    }

    void CreateLogicalDevice(mvGraphicsContext& context)
    {

        QueueFamilyIndices indices = FindQueueFamilies(context.physicalDevice);

        context.graphicsQueueFamily = indices.graphicsFamily.value();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        {
            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos = queueCreateInfos.data();

            createInfo.pEnabledFeatures = &deviceFeatures;

            createInfo.enabledExtensionCount = context.deviceExtensions.size();
            createInfo.ppEnabledExtensionNames = context.deviceExtensions.data();

            if (context.enableValidationLayers)
            {
                createInfo.enabledLayerCount = static_cast<uint32_t>(context.validationLayers.size());
                createInfo.ppEnabledLayerNames = context.validationLayers.data();
            }
            else
                createInfo.enabledLayerCount = 0;

            if (vkCreateDevice(context.physicalDevice, &createInfo, nullptr, &context.logicalDevice) != VK_SUCCESS)
                throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(context.logicalDevice, indices.graphicsFamily.value(), 0, &context.graphicsQueue);
        vkGetDeviceQueue(context.logicalDevice, indices.presentFamily.value(), 0, &context.presentQueue);
    }

    void CreateSwapChain(mvGraphicsContext& context, int width, int height)
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(context.physicalDevice, context.surface);

        // choose swap surface Format
        VkSurfaceFormatKHR surfaceFormat = swapChainSupport.formats[0];
        for (const auto& availableFormat : swapChainSupport.formats)
        {
            if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                surfaceFormat = availableFormat;
                break;
            }
        }

        // chose swap present mode
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        for (const auto& availablePresentMode : swapChainSupport.presentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR)
            {
                presentMode = availablePresentMode;
                break;
            }
        }

        // chose swap extent
        VkExtent2D extent;
        if (swapChainSupport.capabilities.currentExtent.width != UINT32_MAX)
            extent = swapChainSupport.capabilities.currentExtent;
        else
        {
            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::max(swapChainSupport.capabilities.minImageExtent.width, std::min(swapChainSupport.capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(swapChainSupport.capabilities.minImageExtent.height, std::min(swapChainSupport.capabilities.maxImageExtent.height, actualExtent.height));

            extent = actualExtent;
        }

        context.minImageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && context.minImageCount > swapChainSupport.capabilities.maxImageCount)
            context.minImageCount = swapChainSupport.capabilities.maxImageCount;

        {
            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = context.surface;
            createInfo.minImageCount = context.minImageCount;
            createInfo.imageFormat = surfaceFormat.format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            QueueFamilyIndices indices =FindQueueFamilies(context.physicalDevice);
            uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

            if (indices.graphicsFamily != indices.presentFamily)
            {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            }
            else
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

            createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;

            createInfo.oldSwapchain = VK_NULL_HANDLE;

            if (vkCreateSwapchainKHR(context.logicalDevice, &createInfo, nullptr, &context.swapChain) != VK_SUCCESS)
                throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(context.logicalDevice, context.swapChain, &context.minImageCount, nullptr);
        context.swapChainImages.resize(context.minImageCount);
        vkGetSwapchainImagesKHR(context.logicalDevice, context.swapChain, &context.minImageCount, context.swapChainImages.data());

        context.swapChainImageFormat = surfaceFormat.format;
        context.swapChainExtent = extent;
        context.deletionQueue.pushDeletor([=]() { vkDestroySwapchainKHR(context.logicalDevice, context.swapChain, nullptr); });

        // creating image views
        context.swapChainImageViews.resize(context.swapChainImages.size());
        for (uint32_t i = 0; i < context.swapChainImages.size(); i++)
            context.swapChainImageViews[i] = CreateImageView(context.logicalDevice, context.swapChainImages[i], context.swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    void CreateMainCommandPool(mvGraphicsContext& context)
    {
        QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(context.physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(context.logicalDevice, &poolInfo, nullptr, &context.commandPool) != VK_SUCCESS)
            throw std::runtime_error("failed to create command pool!");

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = context.commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)3;

        context.commandBuffers.resize(3);

        if (vkAllocateCommandBuffers(context.logicalDevice, &allocInfo, context.commandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate command buffers!");

        context.deletionQueue.pushDeletor([=]() {vkDestroyCommandPool(context.logicalDevice, context.commandPool, nullptr); });
    }

    void CreateMainDescriptorPool(VkDevice logicalDevice, VkDescriptorPool* descriptorPool)
    {
        VkDescriptorPoolSize poolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };
        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1000 * 11;
        poolInfo.poolSizeCount = 11u;
        poolInfo.pPoolSizes = poolSizes;

        if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, descriptorPool) != VK_SUCCESS)
            throw std::runtime_error("failed to create descriptor pool!");
    }

    void CreateMainRenderPass(VkDevice logicalDevice, VkRenderPass* renderPass, VkFormat format)
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = VK_FORMAT_D32_SFLOAT;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcAccessMask = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, renderPass) != VK_SUCCESS)
            throw std::runtime_error("failed to create render pass!");

        GContext->graphics.deletionQueue.pushDeletor([=]() {
            vkFreeCommandBuffers(logicalDevice,
                GContext->graphics.commandPool,
                static_cast<uint32_t>(GContext->graphics.commandBuffers.size()),
                GContext->graphics.commandBuffers.data());
            vkDestroyRenderPass(logicalDevice, *renderPass, nullptr); }
        );
    }

    void CreateMainDepthResources(VkDevice logicalDevice, VkImage depthImage, VkImageView depthImageView, VkDeviceMemory depthImageMemory)
    {
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

        CreateImage(logicalDevice, GContext->graphics.swapChainExtent.width, GContext->graphics.swapChainExtent.height, depthFormat,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            depthImage, depthImageMemory);

        depthImageView = CreateImageView(logicalDevice, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

        GContext->graphics.deletionQueue.pushDeletor([=]() {
            vkDestroyImageView(logicalDevice, depthImageView, nullptr);
            vkDestroyImage(logicalDevice, depthImage, nullptr);
            vkFreeMemory(logicalDevice, depthImageMemory, nullptr);
            });
    }

    void CreateFrameBuffers(VkDevice logicalDevice, VkRenderPass renderPass, std::vector<VkFramebuffer>& frameBuffers, std::vector<VkImageView>& swapChainImageViews, VkImageView depthImageView)
    {
        frameBuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            VkImageView attachments[] = {
                swapChainImageViews[i],
                depthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 2;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = GContext->graphics.swapChainExtent.width;
            framebufferInfo.height = GContext->graphics.swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to create framebuffer!");

            GContext->graphics.deletionQueue.pushDeletor([=]() {
                vkDestroyFramebuffer(logicalDevice, frameBuffers[i], nullptr);
                vkDestroyImageView(logicalDevice, swapChainImageViews[i], nullptr);
                });
        }
    }

    void CreateSyncObjects(VkDevice logicalDevice, std::vector<VkSemaphore>& imgAvailSema, std::vector<VkSemaphore>& imgFinishSema, std::vector<VkFence>& inFlightFences, std::vector<VkFence>& imagesInFlight)
    {
        imgAvailSema.resize(GContext->graphics.max_frames_in_flight);
        imgFinishSema.resize(GContext->graphics.max_frames_in_flight);
        inFlightFences.resize(GContext->graphics.max_frames_in_flight);
        imagesInFlight.resize(GContext->graphics.swapChainImages.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < GContext->graphics.max_frames_in_flight; i++)
        {
            if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imgAvailSema[i]) != VK_SUCCESS ||
                vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imgFinishSema[i]) != VK_SUCCESS ||
                vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }

    void CreateImage(VkDevice logicalDevice, uint32_t width, uint32_t height, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional

        if (vkCreateImage(logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
            throw std::runtime_error("failed to create image!");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(GContext->graphics.physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate image memory!");

        vkBindImageMemory(logicalDevice, image, imageMemory, 0);
    }

    VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.aspectMask = aspectFlags;

        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
            throw std::runtime_error("failed to create texture image view!");

        return imageView;
    }

    std::uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void CleanupGraphicsContext()
    {
        
        GContext->graphics.deletionQueue.flush();
        mvAllocator::Shutdown();
        vkDestroyDescriptorPool(GContext->graphics.logicalDevice, GContext->graphics.descriptorPool, nullptr);

        for (size_t i = 0; i < GContext->graphics.max_frames_in_flight; i++)
        {
            vkDestroySemaphore(GContext->graphics.logicalDevice, GContext->graphics.imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(GContext->graphics.logicalDevice, GContext->graphics.renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(GContext->graphics.logicalDevice, GContext->graphics.inFlightFences[i], nullptr);
        }

        if (GContext->graphics.enableValidationLayers)
        {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(GetVkInstance(), "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr)
                func(GetVkInstance(), GContext->graphics.debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(GetVkInstance(), GContext->graphics.surface, nullptr);
        vkDestroyDevice(GContext->graphics.logicalDevice, nullptr);
        vkDestroyInstance(GetVkInstance(), nullptr);
    }

    void RecreateSwapChain(int width, int height)
    {
        vkDeviceWaitIdle(GContext->graphics.logicalDevice);

        GContext->graphics.deletionQueue.flush();

        mvAllocator::Shutdown();
        mvAllocator::Init();
        CreateSwapChain(GContext->graphics, width, height);
        CreateMainCommandPool(GContext->graphics);
        CreateMainCommandPool(GContext->graphics);
        CreateMainRenderPass(GContext->graphics.logicalDevice, &(GContext->graphics.renderPass), GContext->graphics.swapChainImageFormat);
        CreateMainDepthResources(GContext->graphics.logicalDevice, GContext->graphics.depthImage, GContext->graphics.depthImageView, GContext->graphics.depthImageMemory);
        CreateFrameBuffers(GContext->graphics.logicalDevice, GContext->graphics.renderPass, GContext->graphics.swapChainFramebuffers, GContext->graphics.swapChainImageViews, GContext->graphics.depthImageView);
    }

    size_t GetRequiredUniformBufferSize(size_t size)
    {
        // Calculate required alignment based on minimum device offset alignment
        size_t minUboAlignment = GContext->graphics.deviceProperties.limits.minUniformBufferOffsetAlignment;
        size_t alignedSize = size;

        if (minUboAlignment > 0)
            alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);

        return alignedSize;
    }
}