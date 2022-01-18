#include "mvGraphics.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <stdexcept>
#include <iostream>
#include <set>
#include <optional>
#include <array>
#include "mvViewport.h"

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
create_image(mvGraphics& graphics, u32 width, u32 height, VkFormat format,
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

    MV_VULKAN(vkCreateImage(graphics.logicalDevice, &imageInfo, nullptr, &image));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(graphics.logicalDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(graphics.physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    MV_VULKAN(vkAllocateMemory(graphics.logicalDevice, &allocInfo, nullptr, &imageMemory));
    MV_VULKAN(vkBindImageMemory(graphics.logicalDevice, image, imageMemory, 0));
}

mv_internal VkImageView
create_image_view(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
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
    MV_VULKAN(vkCreateImageView(device, &viewInfo, nullptr, &imageView));
    return imageView;
}

mv_internal QueueFamilyIndices 
find_queue_families(mvGraphics& graphics, VkPhysicalDevice device)
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
        MV_VULKAN(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, graphics.surface, &presentSupport));

        if (presentSupport)
            indices.presentFamily = i;

        if (indices.isComplete())
            break;

        i++;
    }

    return indices;
}

mv_internal void 
create_swapchain(mvGraphics& graphics, mvViewport& viewport)
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
        MV_VULKAN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics.physicalDevice, graphics.surface, &swapChainSupport.capabilities));

        u32 formatCount;
        MV_VULKAN(vkGetPhysicalDeviceSurfaceFormatsKHR(graphics.physicalDevice, graphics.surface, &formatCount, nullptr));

        // todo: put in appropriate spot
        VkBool32 presentSupport = false;
        MV_VULKAN(vkGetPhysicalDeviceSurfaceSupportKHR(graphics.physicalDevice, 0, graphics.surface, &presentSupport));

        if (formatCount != 0) 
        {
            swapChainSupport.formats.resize(formatCount);
            MV_VULKAN(vkGetPhysicalDeviceSurfaceFormatsKHR(graphics.physicalDevice, graphics.surface, &formatCount, swapChainSupport.formats.data()));
        }

        u32 presentModeCount;
        MV_VULKAN(vkGetPhysicalDeviceSurfacePresentModesKHR(graphics.physicalDevice, graphics.surface, &presentModeCount, nullptr));

        if (presentModeCount != 0) 
        {
            swapChainSupport.presentModes.resize(presentModeCount);
            MV_VULKAN(vkGetPhysicalDeviceSurfacePresentModesKHR(graphics.physicalDevice, graphics.surface, &presentModeCount, swapChainSupport.presentModes.data()));
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
            (u32)viewport.width,
            (u32)viewport.height
        };

        actualExtent.width = std::max(swapChainSupport.capabilities.minImageExtent.width, std::min(swapChainSupport.capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(swapChainSupport.capabilities.minImageExtent.height, std::min(swapChainSupport.capabilities.maxImageExtent.height, actualExtent.height));

        extent = actualExtent;
    }

    graphics.minImageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && graphics.minImageCount > swapChainSupport.capabilities.maxImageCount)
        graphics.minImageCount = swapChainSupport.capabilities.maxImageCount;

    {
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = graphics.surface;
        createInfo.minImageCount = graphics.minImageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = find_queue_families(graphics, graphics.physicalDevice);
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

        MV_VULKAN(vkCreateSwapchainKHR(graphics.logicalDevice, &createInfo, nullptr, &graphics.swapChain));
    }

    vkGetSwapchainImagesKHR(graphics.logicalDevice, graphics.swapChain, &graphics.minImageCount, nullptr);
    graphics.swapChainImages.resize(graphics.minImageCount);
    vkGetSwapchainImagesKHR(graphics.logicalDevice, graphics.swapChain, &graphics.minImageCount, graphics.swapChainImages.data());

    graphics.swapChainImageFormat = surfaceFormat.format;
    graphics.swapChainExtent = extent;

    // creating image views
    graphics.swapChainImageViews.resize(graphics.swapChainImages.size());
    for (u32 i = 0; i < graphics.swapChainImages.size(); i++)
        graphics.swapChainImageViews[i] = create_image_view(graphics.logicalDevice, graphics.swapChainImages[i], graphics.swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

mv_internal void
setup_imgui(mvGraphics& graphics, GLFWwindow* window)
{

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = graphics.instance;
    init_info.PhysicalDevice = graphics.physicalDevice;
    init_info.Device = graphics.logicalDevice;
    init_info.QueueFamily = graphics.graphicsQueueFamily;
    init_info.Queue = graphics.graphicsQueue;
    init_info.PipelineCache = nullptr;
    init_info.DescriptorPool = graphics.descriptorPool;
    init_info.Allocator = nullptr;
    init_info.MinImageCount = graphics.minImageCount;
    init_info.ImageCount = graphics.minImageCount;
    init_info.CheckVkResultFn = nullptr;
    ImGui_ImplVulkan_Init(&init_info, graphics.renderPass);

    // Upload Fonts
    {
        // Use any command queue
        VkCommandBuffer command_buffer = begin_command_buffer(graphics);
        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
        submit_command_buffer(graphics, command_buffer);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

}

mv_internal void 
create_render_pass(VkDevice device, VkFormat format, VkFormat depthformat, VkRenderPass* renderPass)
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

    MV_VULKAN(vkCreateRenderPass(device, &renderPassInfo, nullptr, renderPass));
}

mv_internal void
flush_resources(mvGraphics& graphics)
{
    for (u32 i = 0; i < graphics.swapChainFramebuffers.size(); i++)
    {
        vkDestroyImageView(graphics.logicalDevice, graphics.swapChainImageViews[i], nullptr);
        vkDestroyFramebuffer(graphics.logicalDevice, graphics.swapChainFramebuffers[i], nullptr);
    }
    vkFreeMemory(graphics.logicalDevice, graphics.depthImageMemory, nullptr);
    vkDestroyImage(graphics.logicalDevice, graphics.depthImage, nullptr);
    vkDestroyImageView(graphics.logicalDevice, graphics.depthImageView, nullptr);
    vkDestroyRenderPass(graphics.logicalDevice, graphics.renderPass, nullptr);
    vkDestroySwapchainKHR(graphics.logicalDevice, graphics.swapChain, nullptr);
}

//-----------------------------------------------------------------------------
// declarations for public API
//-----------------------------------------------------------------------------

VkCommandBuffer  
get_current_command_buffer(mvGraphics& graphics)
{
    return graphics.commandBuffers[graphics.currentImageIndex];
}

void
setup_graphics_context(mvGraphics& graphics, mvViewport& viewport, std::vector<const char*> validationLayers, std::vector<const char*> deviceExtensions)
{
    //-----------------------------------------------------------------------------
    // create vulkan instance
    //-----------------------------------------------------------------------------
    {
        if (graphics.enableValidationLayers)
        {
            u32 layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (const char* layerName : validationLayers)
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
        if (graphics.enableValidationLayers)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        createInfo.enabledExtensionCount = (u32)extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();

        // Setup debug messenger for vulkan instance
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        if (graphics.enableValidationLayers)
        {
            createInfo.enabledLayerCount = (u32)validationLayers.size();
            createInfo.ppEnabledLayerNames = validationLayers.data();
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

        MV_VULKAN(vkCreateInstance(&createInfo, nullptr, &graphics.instance));
    }

    //-----------------------------------------------------------------------------
    // setup debug messenger
    //-----------------------------------------------------------------------------
    if (graphics.enableValidationLayers)
    {

        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pNext = VK_NULL_HANDLE;

        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(graphics.instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            MV_VULKAN(func(graphics.instance, &createInfo, nullptr, &graphics.debugMessenger));
        }
        else
            throw std::runtime_error("failed to set up debug messenger!");
    }

    //-----------------------------------------------------------------------------
    // create surface
    //-----------------------------------------------------------------------------
    MV_VULKAN(glfwCreateWindowSurface(graphics.instance, viewport.handle, nullptr, &graphics.surface));

    //-----------------------------------------------------------------------------
    // create physical device
    //-----------------------------------------------------------------------------
    {
        u32 deviceCount = 0;
        MV_VULKAN(vkEnumeratePhysicalDevices(graphics.instance, &deviceCount, nullptr));

        if (deviceCount == 0)
            throw std::runtime_error("failed to find GPUs with Vulkan support!");

        //-----------------------------------------------------------------------------
        // check if device is suitable
        //-----------------------------------------------------------------------------
        std::vector<VkPhysicalDevice> devices(deviceCount);
        MV_VULKAN(vkEnumeratePhysicalDevices(graphics.instance, &deviceCount, devices.data()));
        for (const auto& device : devices)
        {
            QueueFamilyIndices indices = find_queue_families(graphics, device);

            //-----------------------------------------------------------------------------
            // check if device supports extensions
            //-----------------------------------------------------------------------------
            b8 extensionsSupported = false;
            {
                u32 extensionCount;
                MV_VULKAN(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));

                std::vector<VkExtensionProperties> availableExtensions(extensionCount);
                MV_VULKAN(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()));

                std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

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

            vkGetPhysicalDeviceProperties(device, &graphics.deviceProperties);

            if (extensionsSupported && swapChainAdequate && graphics.deviceProperties.limits.maxPushConstantsSize >= 256)
            {
                graphics.physicalDevice = device;
                // TODO: add logic to pick best device (not the last device)
            }
        }

        if (graphics.physicalDevice == VK_NULL_HANDLE)
            throw std::runtime_error("failed to find a suitable GPU!");
    }

    //-----------------------------------------------------------------------------
    // create logical device
    //-----------------------------------------------------------------------------
    {
        QueueFamilyIndices indices = find_queue_families(graphics, graphics.physicalDevice);

        graphics.graphicsQueueFamily = indices.graphicsFamily.value();

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

            createInfo.enabledExtensionCount = (u32)deviceExtensions.size();
            createInfo.ppEnabledExtensionNames = deviceExtensions.data();

            if (graphics.enableValidationLayers)
            {
                createInfo.enabledLayerCount = (u32)validationLayers.size();
                createInfo.ppEnabledLayerNames = validationLayers.data();
            }
            else
                createInfo.enabledLayerCount = 0;

            MV_VULKAN(vkCreateDevice(graphics.physicalDevice, &createInfo, nullptr, &graphics.logicalDevice));
        }

        vkGetDeviceQueue(graphics.logicalDevice, indices.graphicsFamily.value(), 0, &graphics.graphicsQueue);
        vkGetDeviceQueue(graphics.logicalDevice, indices.presentFamily.value(), 0, &graphics.presentQueue);
    }

    //-----------------------------------------------------------------------------
    // create swapchain
    //-----------------------------------------------------------------------------
    create_swapchain(graphics, viewport);

    //-----------------------------------------------------------------------------
    // create command pool and command buffers
    //-----------------------------------------------------------------------------
    QueueFamilyIndices queueFamilyIndices = find_queue_families(graphics, graphics.physicalDevice);

    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    MV_VULKAN(vkCreateCommandPool(graphics.logicalDevice, &commandPoolInfo, nullptr, &graphics.commandPool));

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = graphics.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (u32)(graphics.swapChainImages.size());

    graphics.commandBuffers.resize(graphics.swapChainImages.size());

    MV_VULKAN(vkAllocateCommandBuffers(graphics.logicalDevice, &allocInfo, graphics.commandBuffers.data()));

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

    MV_VULKAN(vkCreateDescriptorPool(graphics.logicalDevice, &descPoolInfo, nullptr, &graphics.descriptorPool));
    
    //-----------------------------------------------------------------------------
    // create render pass
    //-----------------------------------------------------------------------------
    create_render_pass(graphics.logicalDevice, graphics.swapChainImageFormat, VK_FORMAT_D32_SFLOAT, &graphics.renderPass);
   
    //-----------------------------------------------------------------------------
    // create depth resources
    //-----------------------------------------------------------------------------
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

    create_image(graphics, graphics.swapChainExtent.width, graphics.swapChainExtent.height, depthFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        graphics.depthImage, graphics.depthImageMemory);

    graphics.depthImageView = create_image_view(graphics.logicalDevice, graphics.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    //-----------------------------------------------------------------------------
    // create frame buffers
    //-----------------------------------------------------------------------------
    graphics.swapChainFramebuffers.resize(graphics.swapChainImageViews.size());
    for (u32 i = 0; i < graphics.swapChainImageViews.size(); i++)
    {
        VkImageView imageViews[] = { graphics.swapChainImageViews[i], graphics.depthImageView };
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = graphics.renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = imageViews;
        framebufferInfo.width = graphics.swapChainExtent.width;
        framebufferInfo.height = graphics.swapChainExtent.height;
        framebufferInfo.layers = 1;
        MV_VULKAN(vkCreateFramebuffer(graphics.logicalDevice, &framebufferInfo, nullptr, &graphics.swapChainFramebuffers[i]));
    }


    //-----------------------------------------------------------------------------
    // create syncronization primitives
    //-----------------------------------------------------------------------------
    graphics.imageAvailableSemaphores.resize(MV_MAX_FRAMES_IN_FLIGHT);
    graphics.renderFinishedSemaphores.resize(MV_MAX_FRAMES_IN_FLIGHT);
    graphics.inFlightFences.resize(MV_MAX_FRAMES_IN_FLIGHT);
    graphics.imagesInFlight.resize(graphics.swapChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (u32 i = 0; i < MV_MAX_FRAMES_IN_FLIGHT; i++)
    {
        MV_VULKAN(vkCreateSemaphore(graphics.logicalDevice, &semaphoreInfo, nullptr, &graphics.imageAvailableSemaphores[i]));
        MV_VULKAN(vkCreateSemaphore(graphics.logicalDevice, &semaphoreInfo, nullptr, &graphics.renderFinishedSemaphores[i]));
        MV_VULKAN(vkCreateFence(graphics.logicalDevice, &fenceInfo, nullptr, &graphics.inFlightFences[i]));
    }

    //-----------------------------------------------------------------------------
    // Dear ImGui
    //-----------------------------------------------------------------------------
    setup_imgui(graphics, viewport.handle);
}

u32
find_memory_type(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
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
cleanup_graphics_context(mvGraphics& graphics)
{        
    flush_resources(graphics);

    vkFreeCommandBuffers(graphics.logicalDevice,
        graphics.commandPool,
        (u32)graphics.commandBuffers.size(),
        graphics.commandBuffers.data());

    vkDestroyCommandPool(graphics.logicalDevice, graphics.commandPool, nullptr);
    vkDestroyDescriptorPool(graphics.logicalDevice, graphics.descriptorPool, nullptr);

    for (u32 i = 0; i < MV_MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(graphics.logicalDevice, graphics.imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(graphics.logicalDevice, graphics.renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(graphics.logicalDevice, graphics.inFlightFences[i], nullptr);
    }

    if (graphics.enableValidationLayers)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(graphics.instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
            func(graphics.instance, graphics.debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(graphics.instance, graphics.surface, nullptr);
    vkDestroyDevice(graphics.logicalDevice, nullptr);
    vkDestroyInstance(graphics.instance, nullptr);
}

void 
recreate_swapchain(mvGraphics& graphics, mvViewport& viewport)
{
    vkDeviceWaitIdle(graphics.logicalDevice);
    flush_resources(graphics);
    create_swapchain(graphics, viewport);
    create_render_pass(graphics.logicalDevice, graphics.swapChainImageFormat, VK_FORMAT_D32_SFLOAT, &graphics.renderPass);

    //-----------------------------------------------------------------------------
    // create depth resources
    //-----------------------------------------------------------------------------
    {
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

        create_image(graphics, graphics.swapChainExtent.width, graphics.swapChainExtent.height, depthFormat,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            graphics.depthImage, graphics.depthImageMemory);

        graphics.depthImageView = create_image_view(graphics.logicalDevice, graphics.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    graphics.swapChainFramebuffers.resize(graphics.swapChainImageViews.size());
    for (u32 i = 0; i < graphics.swapChainImageViews.size(); i++)
    {
        VkImageView imageViews[] = { graphics.swapChainImageViews[i], graphics.depthImageView };
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = graphics.renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = imageViews;
        framebufferInfo.width = graphics.swapChainExtent.width;
        framebufferInfo.height = graphics.swapChainExtent.height;
        framebufferInfo.layers = 1;
        MV_VULKAN(vkCreateFramebuffer(graphics.logicalDevice, &framebufferInfo, nullptr, &graphics.swapChainFramebuffers[i]));
    }

    ImGui_ImplVulkan_SetMinImageCount(graphics.minImageCount);
}

size_t 
get_required_uniform_buffer_size(mvGraphics& graphics, size_t size)
{
    // Calculate required alignment based on minimum device offset alignment
    size_t minUboAlignment = graphics.deviceProperties.limits.minUniformBufferOffsetAlignment;
    size_t alignedSize = size;

    if (minUboAlignment > 0)
        alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);

    return alignedSize;
}

void
present(mvGraphics& graphics)
{

    VkSemaphore signalSemaphores[] = { graphics.renderFinishedSemaphores[graphics.currentFrame] };

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { graphics.swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &graphics.currentImageIndex;

    VkResult result = vkQueuePresentKHR(graphics.presentQueue, &presentInfo);

    graphics.currentFrame = (graphics.currentFrame + 1) % MV_MAX_FRAMES_IN_FLIGHT;
}

VkCommandBuffer 
begin_command_buffer(mvGraphics& graphics)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = graphics.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(graphics.logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void 
submit_command_buffer(mvGraphics& graphics, VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphics.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkDeviceWaitIdle(graphics.logicalDevice);

    vkFreeCommandBuffers(graphics.logicalDevice, graphics.commandPool, 1, &commandBuffer);
}
