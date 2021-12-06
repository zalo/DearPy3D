#include "mvGraphics.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <stdexcept>
#include <iostream>
#include <set>
#include <optional>
#include <array>
#include "mvContext.h"

mv_internal VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
    VkDebugUtilsMessageTypeFlagsEXT messageType, 
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}


//-----------------------------------------------------------------------------
// internals
//-----------------------------------------------------------------------------

struct QueueFamilyIndices
{
    std::optional<u32> graphicsFamily;
    std::optional<u32> presentFamily;

    bool isComplete() 
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

mv_internal void
mvCreateImage(u32 width, u32 height, VkFormat format,
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

    MV_VULKAN(vkCreateImage(mvGetLogicalDevice(), &imageInfo, nullptr, &image));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(mvGetLogicalDevice(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = mvFindMemoryType(GContext->graphics.physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    MV_VULKAN(vkAllocateMemory(mvGetLogicalDevice(), &allocInfo, nullptr, &imageMemory));
    MV_VULKAN(vkBindImageMemory(mvGetLogicalDevice(), image, imageMemory, 0));
}

mv_internal VkImageView
mvCreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
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
    MV_VULKAN(vkCreateImageView(mvGetLogicalDevice(), &viewInfo, nullptr, &imageView));
    return imageView;
}

mv_internal QueueFamilyIndices 
mvFindQueueFamilies(VkPhysicalDevice device)
{

    QueueFamilyIndices indices;

    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) 
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        VkBool32 presentSupport = false;
        MV_VULKAN(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, GContext->graphics.surface, &presentSupport));

        if (presentSupport)
            indices.presentFamily = i;

        if (indices.isComplete())
            break;

        i++;
    }

    return indices;
}

mv_internal void 
mvCreateSwapChain()
{

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };
   
    //-----------------------------------------------------------------------------
    // query swapchain support
    //-----------------------------------------------------------------------------
    SwapChainSupportDetails swapChainSupport;
    {
        MV_VULKAN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GContext->graphics.physicalDevice, GContext->graphics.surface, &swapChainSupport.capabilities));

        u32 formatCount;
        MV_VULKAN(vkGetPhysicalDeviceSurfaceFormatsKHR(GContext->graphics.physicalDevice, GContext->graphics.surface, &formatCount, nullptr));

        // todo: put in appropriate spot
        VkBool32 presentSupport = false;
        MV_VULKAN(vkGetPhysicalDeviceSurfaceSupportKHR(GContext->graphics.physicalDevice, 0, GContext->graphics.surface, &presentSupport));

        if (formatCount != 0) 
        {
            swapChainSupport.formats.resize(formatCount);
            MV_VULKAN(vkGetPhysicalDeviceSurfaceFormatsKHR(GContext->graphics.physicalDevice, GContext->graphics.surface, &formatCount, swapChainSupport.formats.data()));
        }

        u32 presentModeCount;
        MV_VULKAN(vkGetPhysicalDeviceSurfacePresentModesKHR(GContext->graphics.physicalDevice, GContext->graphics.surface, &presentModeCount, nullptr));

        if (presentModeCount != 0) 
        {
            swapChainSupport.presentModes.resize(presentModeCount);
            MV_VULKAN(vkGetPhysicalDeviceSurfacePresentModesKHR(GContext->graphics.physicalDevice, GContext->graphics.surface, &presentModeCount, swapChainSupport.presentModes.data()));
        }
    }

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
            (u32)GContext->viewport.width,
            (u32)GContext->viewport.height
        };

        actualExtent.width = std::max(swapChainSupport.capabilities.minImageExtent.width, std::min(swapChainSupport.capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(swapChainSupport.capabilities.minImageExtent.height, std::min(swapChainSupport.capabilities.maxImageExtent.height, actualExtent.height));

        extent = actualExtent;
    }

    GContext->graphics.minImageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && GContext->graphics.minImageCount > swapChainSupport.capabilities.maxImageCount)
        GContext->graphics.minImageCount = swapChainSupport.capabilities.maxImageCount;

    {
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = GContext->graphics.surface;
        createInfo.minImageCount = GContext->graphics.minImageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = mvFindQueueFamilies(GContext->graphics.physicalDevice);
        u32 queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

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

        MV_VULKAN(vkCreateSwapchainKHR(mvGetLogicalDevice(), &createInfo, nullptr, &GContext->graphics.swapChain));
    }

    vkGetSwapchainImagesKHR(mvGetLogicalDevice(), GContext->graphics.swapChain, &GContext->graphics.minImageCount, nullptr);
    GContext->graphics.swapChainImages.resize(GContext->graphics.minImageCount);
    vkGetSwapchainImagesKHR(mvGetLogicalDevice(), GContext->graphics.swapChain, &GContext->graphics.minImageCount, GContext->graphics.swapChainImages.data());

    GContext->graphics.swapChainImageFormat = surfaceFormat.format;
    GContext->graphics.swapChainExtent = extent;

    // creating image views
    GContext->graphics.swapChainImageViews.resize(GContext->graphics.swapChainImages.size());
    for (u32 i = 0; i < GContext->graphics.swapChainImages.size(); i++)
        GContext->graphics.swapChainImageViews[i] = mvCreateImageView(GContext->graphics.swapChainImages[i], GContext->graphics.swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

mv_internal void
mvSetupImGui(GLFWwindow* window)
{

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = GContext->graphics.instance;
    init_info.PhysicalDevice = GContext->graphics.physicalDevice;
    init_info.Device = GContext->graphics.logicalDevice;
    init_info.QueueFamily = GContext->graphics.graphicsQueueFamily;
    init_info.Queue = GContext->graphics.graphicsQueue;
    init_info.PipelineCache = nullptr;
    init_info.DescriptorPool = GContext->graphics.descriptorPool;
    init_info.Allocator = nullptr;
    init_info.MinImageCount = GContext->graphics.minImageCount;
    init_info.ImageCount = GContext->graphics.minImageCount;
    init_info.CheckVkResultFn = nullptr;
    ImGui_ImplVulkan_Init(&init_info, GContext->graphics.renderPass);

    // Upload Fonts
    {
        // Use any command queue
        VkCommandBuffer command_buffer = mvBeginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
        mvEndSingleTimeCommands(command_buffer);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

}

mv_internal void 
mvCreateRenderPass(VkFormat format, VkFormat depthformat, VkRenderPass* renderPass)
{
    VkAttachmentDescription attachments[2];

    // color attachment
    attachments[0].flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
    attachments[0].format = format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // depth attachment
    attachments[1].flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
    attachments[1].format = depthformat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference references[] = 
    {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}
    };

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = references;
    subpass.pDepthStencilAttachment = &references[1];

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2u;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 0;
    renderPassInfo.pDependencies = VK_NULL_HANDLE;

    MV_VULKAN(vkCreateRenderPass(mvGetLogicalDevice(), &renderPassInfo, nullptr, renderPass));
}

mv_internal void
mvFlushResources()
{
    for (u32 i = 0; i < GContext->graphics.swapChainFramebuffers.size(); i++)
    {
        vkDestroyImageView(mvGetLogicalDevice(), GContext->graphics.swapChainImageViews[i], nullptr);
        vkDestroyFramebuffer(mvGetLogicalDevice(), GContext->graphics.swapChainFramebuffers[i], nullptr);
    }
    vkFreeMemory(mvGetLogicalDevice(), GContext->graphics.depthImageMemory, nullptr);
    vkDestroyImage(mvGetLogicalDevice(), GContext->graphics.depthImage, nullptr);
    vkDestroyImageView(mvGetLogicalDevice(), GContext->graphics.depthImageView, nullptr);
    vkDestroyRenderPass(mvGetLogicalDevice(), GContext->graphics.renderPass, nullptr);
    vkDestroySwapchainKHR(mvGetLogicalDevice(), GContext->graphics.swapChain, nullptr);
}

//-----------------------------------------------------------------------------
// declarations for public API
//-----------------------------------------------------------------------------

VkDevice 
mvGetLogicalDevice()
{
    return GContext->graphics.logicalDevice;
}

VkCommandBuffer  
mvGetCurrentCommandBuffer()
{
    return GContext->graphics.commandBuffers[GContext->graphics.currentImageIndex];
}

void 
mvSetupGraphicsContext()
{
    //-----------------------------------------------------------------------------
    // create vulkan instance
    //-----------------------------------------------------------------------------
    {
        if (GContext->graphics.enableValidationLayers)
        {
            u32 layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (const char* layerName : GContext->graphics.validationLayers)
            {
                bool layerFound = false;

                for (const auto& layerProperties : availableLayers)
                {
                    if (strcmp(layerName, layerProperties.layerName) == 0)
                    {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound)
                    throw std::runtime_error("validation layers requested, but not available!");
            }
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.apiVersion = VK_API_VERSION_1_2;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // get extensions required to load glfw
        u32 glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (GContext->graphics.enableValidationLayers)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        createInfo.enabledExtensionCount = (u32)extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();

        // Setup debug messenger for vulkan instance
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        if (GContext->graphics.enableValidationLayers)
        {
            createInfo.enabledLayerCount = (u32)GContext->graphics.validationLayers.size();
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

        MV_VULKAN(vkCreateInstance(&createInfo, nullptr, &GContext->graphics.instance));
    }

    //-----------------------------------------------------------------------------
    // setup debug messenger
    //-----------------------------------------------------------------------------
    if (GContext->graphics.enableValidationLayers)
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
            MV_VULKAN(func(GContext->graphics.instance, &createInfo, nullptr, &GContext->graphics.debugMessenger));
        }
        else
            throw std::runtime_error("failed to set up debug messenger!");
    }

    //-----------------------------------------------------------------------------
    // create surface
    //-----------------------------------------------------------------------------
    MV_VULKAN(glfwCreateWindowSurface(GContext->graphics.instance, GContext->viewport.handle, nullptr, &GContext->graphics.surface));

    //-----------------------------------------------------------------------------
    // create physical device
    //-----------------------------------------------------------------------------
    {
        u32 deviceCount = 0;
        MV_VULKAN(vkEnumeratePhysicalDevices(GContext->graphics.instance, &deviceCount, nullptr));

        if (deviceCount == 0)
            throw std::runtime_error("failed to find GPUs with Vulkan support!");

        //-----------------------------------------------------------------------------
        // check if device is suitable
        //-----------------------------------------------------------------------------
        std::vector<VkPhysicalDevice> devices(deviceCount);
        MV_VULKAN(vkEnumeratePhysicalDevices(GContext->graphics.instance, &deviceCount, devices.data()));
        for (const auto& device : devices)
        {
            QueueFamilyIndices indices = mvFindQueueFamilies(device);

            //-----------------------------------------------------------------------------
            // check if device supports extensions
            //-----------------------------------------------------------------------------
            b8 extensionsSupported = false;
            {
                u32 extensionCount;
                MV_VULKAN(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));

                std::vector<VkExtensionProperties> availableExtensions(extensionCount);
                MV_VULKAN(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()));

                std::set<std::string> requiredExtensions(GContext->graphics.deviceExtensions.begin(), GContext->graphics.deviceExtensions.end());

                for (const auto& extension : availableExtensions)
                    requiredExtensions.erase(extension.extensionName);

                extensionsSupported = requiredExtensions.empty();
            }


            bool swapChainAdequate = false;
            if (extensionsSupported)
            {
                swapChainAdequate = true;
                //SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
                //swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }

            vkGetPhysicalDeviceProperties(device, &GContext->graphics.deviceProperties);

            if (extensionsSupported && swapChainAdequate && GContext->graphics.deviceProperties.limits.maxPushConstantsSize >= 256)
            {
                GContext->graphics.physicalDevice = device;
                // TODO: add logic to pick best device (not the last device)
            }
        }

        if (GContext->graphics.physicalDevice == VK_NULL_HANDLE)
            throw std::runtime_error("failed to find a suitable GPU!");
    }

    //-----------------------------------------------------------------------------
    // create logical device
    //-----------------------------------------------------------------------------
    {
        QueueFamilyIndices indices = mvFindQueueFamilies(GContext->graphics.physicalDevice);

        GContext->graphics.graphicsQueueFamily = indices.graphicsFamily.value();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<u32> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        f32 queuePriority = 1.0f;
        for (u32 queueFamily : uniqueQueueFamilies)
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

            createInfo.queueCreateInfoCount = (u32)queueCreateInfos.size();
            createInfo.pQueueCreateInfos = queueCreateInfos.data();

            createInfo.pEnabledFeatures = &deviceFeatures;

            createInfo.enabledExtensionCount = (u32)GContext->graphics.deviceExtensions.size();
            createInfo.ppEnabledExtensionNames = GContext->graphics.deviceExtensions.data();

            if (GContext->graphics.enableValidationLayers)
            {
                createInfo.enabledLayerCount = (u32)GContext->graphics.validationLayers.size();
                createInfo.ppEnabledLayerNames = GContext->graphics.validationLayers.data();
            }
            else
                createInfo.enabledLayerCount = 0;

            MV_VULKAN(vkCreateDevice(GContext->graphics.physicalDevice, &createInfo, nullptr, &GContext->graphics.logicalDevice));
        }

        vkGetDeviceQueue(mvGetLogicalDevice(), indices.graphicsFamily.value(), 0, &GContext->graphics.graphicsQueue);
        vkGetDeviceQueue(mvGetLogicalDevice(), indices.presentFamily.value(), 0, &GContext->graphics.presentQueue);
    }

    //-----------------------------------------------------------------------------
    // create swapchain
    //-----------------------------------------------------------------------------
    mvCreateSwapChain();

    //-----------------------------------------------------------------------------
    // create command pool and command buffers
    //-----------------------------------------------------------------------------
    QueueFamilyIndices queueFamilyIndices = mvFindQueueFamilies(GContext->graphics.physicalDevice);

    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    MV_VULKAN(vkCreateCommandPool(mvGetLogicalDevice(), &commandPoolInfo, nullptr, &GContext->graphics.commandPool));

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = GContext->graphics.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (u32)(GContext->graphics.swapChainImages.size());

    GContext->graphics.commandBuffers.resize(GContext->graphics.swapChainImages.size());

    MV_VULKAN(vkAllocateCommandBuffers(mvGetLogicalDevice(), &allocInfo, GContext->graphics.commandBuffers.data()));

    //-----------------------------------------------------------------------------
    // create descriptor pool
    //-----------------------------------------------------------------------------
    VkDescriptorPoolSize poolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER,                1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000 }
    };
    VkDescriptorPoolCreateInfo descPoolInfo = {};
    descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descPoolInfo.maxSets = 1000 * 11;
    descPoolInfo.poolSizeCount = 11u;
    descPoolInfo.pPoolSizes = poolSizes;

    MV_VULKAN(vkCreateDescriptorPool(mvGetLogicalDevice(), &descPoolInfo, nullptr, &GContext->graphics.descriptorPool));
    
    //-----------------------------------------------------------------------------
    // create render pass
    //-----------------------------------------------------------------------------
    mvCreateRenderPass(GContext->graphics.swapChainImageFormat, VK_FORMAT_D32_SFLOAT, &GContext->graphics.renderPass);
   
    //-----------------------------------------------------------------------------
    // create depth resources
    //-----------------------------------------------------------------------------
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

    mvCreateImage(GContext->graphics.swapChainExtent.width, GContext->graphics.swapChainExtent.height, depthFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        GContext->graphics.depthImage, GContext->graphics.depthImageMemory);

    GContext->graphics.depthImageView = mvCreateImageView(GContext->graphics.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    //-----------------------------------------------------------------------------
    // create frame buffers
    //-----------------------------------------------------------------------------
    GContext->graphics.swapChainFramebuffers.resize(GContext->graphics.swapChainImageViews.size());
    for (u32 i = 0; i < GContext->graphics.swapChainImageViews.size(); i++)
    {
        VkImageView imageViews[] = { GContext->graphics.swapChainImageViews[i], GContext->graphics.depthImageView };
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = GContext->graphics.renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = imageViews;
        framebufferInfo.width = GContext->graphics.swapChainExtent.width;
        framebufferInfo.height = GContext->graphics.swapChainExtent.height;
        framebufferInfo.layers = 1;
        MV_VULKAN(vkCreateFramebuffer(mvGetLogicalDevice(), &framebufferInfo, nullptr, &GContext->graphics.swapChainFramebuffers[i]));
    }


    //-----------------------------------------------------------------------------
    // create syncronization primitives
    //-----------------------------------------------------------------------------
    GContext->graphics.imageAvailableSemaphores.resize(MV_MAX_FRAMES_IN_FLIGHT);
    GContext->graphics.renderFinishedSemaphores.resize(MV_MAX_FRAMES_IN_FLIGHT);
    GContext->graphics.inFlightFences.resize(MV_MAX_FRAMES_IN_FLIGHT);
    GContext->graphics.imagesInFlight.resize(GContext->graphics.swapChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (u32 i = 0; i < MV_MAX_FRAMES_IN_FLIGHT; i++)
    {
        MV_VULKAN(vkCreateSemaphore(mvGetLogicalDevice(), &semaphoreInfo, nullptr, &GContext->graphics.imageAvailableSemaphores[i]));
        MV_VULKAN(vkCreateSemaphore(mvGetLogicalDevice(), &semaphoreInfo, nullptr, &GContext->graphics.renderFinishedSemaphores[i]));
        MV_VULKAN(vkCreateFence(mvGetLogicalDevice(), &fenceInfo, nullptr, &GContext->graphics.inFlightFences[i]));
    }

    //-----------------------------------------------------------------------------
    // Dear ImGui
    //-----------------------------------------------------------------------------
    mvSetupImGui(GContext->viewport.handle);
}

u32
mvFindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void 
mvCleanupGraphicsContext()
{
    // cleanup imgui
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
        
    mvFlushResources();

    vkFreeCommandBuffers(mvGetLogicalDevice(),
        GContext->graphics.commandPool,
        (u32)GContext->graphics.commandBuffers.size(),
        GContext->graphics.commandBuffers.data());

    vkDestroyCommandPool(mvGetLogicalDevice(), GContext->graphics.commandPool, nullptr);
    vkDestroyDescriptorPool(GContext->graphics.logicalDevice, GContext->graphics.descriptorPool, nullptr);

    for (u32 i = 0; i < MV_MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(GContext->graphics.logicalDevice, GContext->graphics.imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(GContext->graphics.logicalDevice, GContext->graphics.renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(GContext->graphics.logicalDevice, GContext->graphics.inFlightFences[i], nullptr);
    }

    if (GContext->graphics.enableValidationLayers)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(GContext->graphics.instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
            func(GContext->graphics.instance, GContext->graphics.debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(GContext->graphics.instance, GContext->graphics.surface, nullptr);
    vkDestroyDevice(GContext->graphics.logicalDevice, nullptr);
    vkDestroyInstance(GContext->graphics.instance, nullptr);
}

void 
mvRecreateSwapChain()
{
    vkDeviceWaitIdle(mvGetLogicalDevice());
    mvFlushResources();
    mvCreateSwapChain();
    mvCreateRenderPass(GContext->graphics.swapChainImageFormat, VK_FORMAT_D32_SFLOAT, &GContext->graphics.renderPass);

    //-----------------------------------------------------------------------------
    // create depth resources
    //-----------------------------------------------------------------------------
    {
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

        mvCreateImage(GContext->graphics.swapChainExtent.width, GContext->graphics.swapChainExtent.height, depthFormat,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            GContext->graphics.depthImage, GContext->graphics.depthImageMemory);

        GContext->graphics.depthImageView = mvCreateImageView(GContext->graphics.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    GContext->graphics.swapChainFramebuffers.resize(GContext->graphics.swapChainImageViews.size());
    for (u32 i = 0; i < GContext->graphics.swapChainImageViews.size(); i++)
    {
        VkImageView imageViews[] = { GContext->graphics.swapChainImageViews[i], GContext->graphics.depthImageView };
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = GContext->graphics.renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = imageViews;
        framebufferInfo.width = GContext->graphics.swapChainExtent.width;
        framebufferInfo.height = GContext->graphics.swapChainExtent.height;
        framebufferInfo.layers = 1;
        MV_VULKAN(vkCreateFramebuffer(mvGetLogicalDevice(), &framebufferInfo, nullptr, &GContext->graphics.swapChainFramebuffers[i]));
    }

    ImGui_ImplVulkan_SetMinImageCount(GContext->graphics.minImageCount);
}

size_t 
mvGetRequiredUniformBufferSize(size_t size)
{
    // Calculate required alignment based on minimum device offset alignment
    size_t minUboAlignment = GContext->graphics.deviceProperties.limits.minUniformBufferOffsetAlignment;
    size_t alignedSize = size;

    if (minUboAlignment > 0)
        alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);

    return alignedSize;
}

void
mvPresent()
{

    VkSemaphore signalSemaphores[] = { GContext->graphics.renderFinishedSemaphores[GContext->graphics.currentFrame] };

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { GContext->graphics.swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &GContext->graphics.currentImageIndex;

    VkResult result = vkQueuePresentKHR(GContext->graphics.presentQueue, &presentInfo);

    GContext->graphics.currentFrame = (GContext->graphics.currentFrame + 1) % MV_MAX_FRAMES_IN_FLIGHT;
}

VkCommandBuffer 
mvBeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = GContext->graphics.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(mvGetLogicalDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void 
mvEndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(GContext->graphics.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkDeviceWaitIdle(mvGetLogicalDevice());

    vkFreeCommandBuffers(mvGetLogicalDevice(), GContext->graphics.commandPool, 1, &commandBuffer);
}
