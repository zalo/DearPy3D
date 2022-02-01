#include "mvTextures.h"
#include <stdexcept>
#include "imgui_impl_vulkan.h"
#include "sMath.h"
#include "mvBuffer.h"
#include "mvGraphics.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void 
DearPy3D::transition_image_layout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange,
    VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
    //VkCommandBuffer commandBuffer = mvBeginSingleTimeCommands();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = subresourceRange;

    // Source layouts (old)
    // Source access mask controls actions that have to be finished on the old layout
    // before it will be transitioned to the new layout
    switch (oldLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        // Image layout is undefined (or does not matter)
        // Only valid as initial layout
        // No flags required, listed only for completeness
        barrier.srcAccessMask = 0;
        break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        // Image is preinitialized
        // Only valid as initial layout for linear images, preserves memory contents
        // Make sure host writes have been finished
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image is a color attachment
        // Make sure any writes to the color buffer have been finished
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image is a depth/stencil attachment
        // Make sure any writes to the depth/stencil buffer have been finished
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image is a transfer source
        // Make sure any reads from the image have been finished
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image is a transfer destination
        // Make sure any writes to the image have been finished
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image is read by a shader
        // Make sure any shader reads from the image have been finished
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch (newLayout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image will be used as a transfer destination
        // Make sure any writes to the image have been finished
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image will be used as a transfer source
        // Make sure any reads from the image have been finished
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image will be used as a color attachment
        // Make sure any writes to the color buffer have been finished
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image layout will be used as a depth/stencil attachment
        // Make sure any writes to depth/stencil buffer have been finished
        barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image will be read in a shader (sampler, input attachment)
        // Make sure any writes to the image have been finished
        if (barrier.srcAccessMask == 0)
        {
            barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        srcStageMask, dstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    //mvEndSingleTimeCommands(commandBuffer);
}

void 
DearPy3D::transition_image_layout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout,
    VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
    VkImageSubresourceRange subresourceRange{};
    subresourceRange.baseMipLevel = 0;
    subresourceRange.layerCount = 1;
    subresourceRange.aspectMask = aspectMask;
    subresourceRange.levelCount = 1;
    transition_image_layout(commandBuffer, image, oldLayout, newLayout, subresourceRange, srcStageMask, dstStageMask);
}

mv_internal void 
mvGenerateMipmaps(mvGraphics& graphics, VkImage image, VkFormat imageFormat, i32 texWidth, i32 texHeight, u32 mipLevels, u32 layers = 1u)
{
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(graphics.physicalDevice, imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = DearPy3D::begin_command_buffer(graphics);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layers;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++) 
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    DearPy3D::submit_command_buffer(graphics, commandBuffer);
}

mvTexture
DearPy3D::create_texture_cube(mvGraphics& graphics, const char* path)
{
    mvTexture texture;
    texture.file = path;
    texture.imageInfo = VkDescriptorImageInfo{};
    texture.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    texture.textureImage = VK_NULL_HANDLE;
    texture.textureImageMemory = VK_NULL_HANDLE;

    // Load Image
    int texWidth, texHeight, texNumChannels, textBytesPerRow;

    // load 6 surfaces for cube faces
    std::vector<unsigned char*> surfaces;
    int texForceNumChannels = 4;

    // right
    {
        std::string file = std::string(path) + "\\right.png";
        unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
            &texNumChannels, texForceNumChannels);
        assert(testTextureBytes);
        textBytesPerRow = 4 * texWidth;

        surfaces.push_back(testTextureBytes);
    }

    // left
    {
        std::string file = std::string(path) + "\\left.png";
        unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
            &texNumChannels, texForceNumChannels);
        assert(testTextureBytes);
        textBytesPerRow = 4 * texWidth;

        surfaces.push_back(testTextureBytes);
    }

    // top
    {
        std::string file = std::string(path) + "\\top.png";
        unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
            &texNumChannels, texForceNumChannels);
        assert(testTextureBytes);
        textBytesPerRow = 4 * texWidth;

        surfaces.push_back(testTextureBytes);
    }

    // bottom
    {
        std::string file = std::string(path) + "\\bottom.png";
        unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
            &texNumChannels, texForceNumChannels);
        assert(testTextureBytes);
        textBytesPerRow = 4 * texWidth;

        surfaces.push_back(testTextureBytes);
    }

    // front
    {
        std::string file = std::string(path) + "\\front.png";
        unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
            &texNumChannels, texForceNumChannels);
        assert(testTextureBytes);
        textBytesPerRow = 4 * texWidth;

        surfaces.push_back(testTextureBytes);
    }

    // back
    {
        std::string file = std::string(path) + "\\back.png";
        unsigned char* testTextureBytes = stbi_load(file.c_str(), &texWidth, &texHeight,
            &texNumChannels, texForceNumChannels);
        assert(testTextureBytes);
        textBytesPerRow = 4 * texWidth;

        surfaces.push_back(testTextureBytes);
    }

    VkDeviceSize imageSize = texWidth * texHeight * 4 * 6;
    VkDeviceSize layerSize = imageSize / 6;

    //texture.mipLevels = (u32)mvFloor(mvLog2(mvMax(texWidth, texHeight))) + 1u;
    texture.mipLevels = 1u;

    //-----------------------------------------------------------------------------
    // staging buffer
    //-----------------------------------------------------------------------------
    mvBufferSpecification bufferSpec{};
    bufferSpec.size = imageSize;
    bufferSpec.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferSpec.propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    mvBuffer stagingBuffer = create_buffer(graphics, bufferSpec);

    void* mapping;
    MV_VULKAN(vkMapMemory(graphics.logicalDevice, stagingBuffer.deviceMemory, 0, stagingBuffer.actualSize, 0, &mapping));
    for (u32 i = 0; i < 6; ++i)
    {
        char* data = (char*)mapping;
        memcpy(data + layerSize*i, surfaces[i], (size_t)layerSize);
    }
    vkUnmapMemory(graphics.logicalDevice, stagingBuffer.deviceMemory);
    mapping = nullptr;

    for (u32 i = 0; i < 6; ++i)
    {
        stbi_image_free(surfaces[i]);
    }

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = texWidth;
    imageInfo.extent.height = texHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = texture.mipLevels;
    imageInfo.arrayLayers = 6;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    MV_VULKAN(vkCreateImage(graphics.logicalDevice, &imageInfo, nullptr, &texture.textureImage));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(graphics.logicalDevice, texture.textureImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(graphics.physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    MV_VULKAN(vkAllocateMemory(graphics.logicalDevice, &allocInfo, nullptr, &texture.textureImageMemory));
    MV_VULKAN(vkBindImageMemory(graphics.logicalDevice, texture.textureImage, texture.textureImageMemory, 0));

    //-----------------------------------------------------------------------------
    // final image
    //-----------------------------------------------------------------------------

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = imageInfo.mipLevels;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 6;

    VkCommandBuffer commandBuffer = begin_command_buffer(graphics);
    transition_image_layout(commandBuffer, texture.textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
    submit_command_buffer(graphics, commandBuffer);
    copy_buffer_to_image(graphics, stagingBuffer, texture.textureImage, (u32)texWidth, (u32)texHeight, 6);
    //mvTransitionImageLayout(texture.textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, imageInfo.mipLevels);

    vkDestroyBuffer(graphics.logicalDevice, stagingBuffer.buffer, nullptr);
    vkFreeMemory(graphics.logicalDevice, stagingBuffer.deviceMemory, nullptr);

    mvGenerateMipmaps(graphics, texture.textureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, imageInfo.mipLevels, 6);

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(graphics.physicalDevice, &properties);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture.textureImage;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = imageInfo.mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 6;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    MV_VULKAN(vkCreateImageView(graphics.logicalDevice, &viewInfo, nullptr, &texture.imageInfo.imageView));

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = (f32)imageInfo.mipLevels;

    MV_VULKAN(vkCreateSampler(graphics.logicalDevice, &samplerInfo, nullptr, &texture.imageInfo.sampler));
    texture.imguiID = ImGui_ImplVulkan_AddTexture(texture.imageInfo.sampler, texture.imageInfo.imageView, texture.imageInfo.imageLayout);
    return texture;
}

mvTexture 
DearPy3D::create_texture(mvGraphics& graphics, const char* file)
{
    mvTexture texture;
    texture.file = file;
    texture.imageInfo = VkDescriptorImageInfo{};
    texture.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    texture.textureImage = VK_NULL_HANDLE;
    texture.textureImageMemory = VK_NULL_HANDLE;

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(file, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    texture.mipLevels = (u32)Semper::get_floor(Semper::log2(Semper::get_max(texWidth, texHeight))) + 1u;
    //texture.mipLevels = 1u;

    if (!pixels)
        throw std::runtime_error("failed to load texture image!");

    //-----------------------------------------------------------------------------
    // staging buffer
    //-----------------------------------------------------------------------------
    mvBufferSpecification bufferSpec;
    bufferSpec.size = imageSize;
    bufferSpec.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferSpec.propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    mvBuffer stagingBuffer = create_buffer(graphics, bufferSpec);

    void* mapping;
    MV_VULKAN(vkMapMemory(graphics.logicalDevice, stagingBuffer.deviceMemory, 0, stagingBuffer.actualSize, 0, &mapping));
    memcpy(mapping, pixels, (size_t)imageSize);
    vkUnmapMemory(graphics.logicalDevice, stagingBuffer.deviceMemory);
    mapping = nullptr;

    stbi_image_free(pixels);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = texWidth;
    imageInfo.extent.height = texHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = texture.mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;
    MV_VULKAN(vkCreateImage(graphics.logicalDevice, &imageInfo, nullptr, &texture.textureImage));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(graphics.logicalDevice, texture.textureImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(graphics.physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    MV_VULKAN(vkAllocateMemory(graphics.logicalDevice, &allocInfo, nullptr, &texture.textureImageMemory));
    MV_VULKAN(vkBindImageMemory(graphics.logicalDevice, texture.textureImage, texture.textureImageMemory, 0));

    //-----------------------------------------------------------------------------
    // final image
    //-----------------------------------------------------------------------------

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = imageInfo.mipLevels;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;

    VkCommandBuffer commandBuffer = begin_command_buffer(graphics);
    transition_image_layout(commandBuffer, texture.textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
    submit_command_buffer(graphics, commandBuffer);
    copy_buffer_to_image(graphics, stagingBuffer, texture.textureImage, (u32)texWidth, (u32)texHeight);
    //mvTransitionImageLayout(texture.textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, imageInfo.mipLevels);

    vkDestroyBuffer(graphics.logicalDevice, stagingBuffer.buffer, nullptr);
    vkFreeMemory(graphics.logicalDevice, stagingBuffer.deviceMemory, nullptr);

    mvGenerateMipmaps(graphics, texture.textureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, imageInfo.mipLevels);

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(graphics.physicalDevice, &properties);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture.textureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = imageInfo.mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    MV_VULKAN(vkCreateImageView(graphics.logicalDevice, &viewInfo, nullptr, &texture.imageInfo.imageView));

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = (f32)imageInfo.mipLevels;

    MV_VULKAN(vkCreateSampler(graphics.logicalDevice, &samplerInfo, nullptr, &texture.imageInfo.sampler));
    texture.imguiID = ImGui_ImplVulkan_AddTexture(texture.imageInfo.sampler, texture.imageInfo.imageView, texture.imageInfo.imageLayout);
    return texture;
}

mvTexture
DearPy3D::create_texture(mvGraphics& graphics, u32 width, u32 height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect)
{
    mvTexture texture;
    texture.imageInfo = VkDescriptorImageInfo{};
    texture.textureImage = VK_NULL_HANDLE;
    texture.textureImageMemory = VK_NULL_HANDLE;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    MV_VULKAN(vkCreateImage(graphics.logicalDevice, &imageInfo, nullptr, &texture.textureImage));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(graphics.logicalDevice, texture.textureImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(graphics.physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    MV_VULKAN(vkAllocateMemory(graphics.logicalDevice, &allocInfo, nullptr, &texture.textureImageMemory));
    MV_VULKAN(vkBindImageMemory(graphics.logicalDevice, texture.textureImage, texture.textureImageMemory, 0));

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture.textureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.aspectMask = aspect;
    MV_VULKAN(vkCreateImageView(graphics.logicalDevice, &viewInfo, nullptr, &texture.imageInfo.imageView));

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = samplerInfo.addressModeU;
    samplerInfo.addressModeW = samplerInfo.addressModeU;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    MV_VULKAN(vkCreateSampler(graphics.logicalDevice, &samplerInfo, nullptr, &texture.imageInfo.sampler));

    if (format == VK_FORMAT_R8G8B8A8_UNORM)
    {
        texture.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        texture.imguiID = ImGui_ImplVulkan_AddTexture(texture.imageInfo.sampler, texture.imageInfo.imageView, texture.imageInfo.imageLayout);
    }
    else
    {
        texture.imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    }

    return texture;
}

mvTexture
DearPy3D::create_texture_cube(mvGraphics& graphics, u32 width, u32 height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect)
{
    mvTexture texture;
    texture.imageInfo = VkDescriptorImageInfo{};
    texture.textureImage = VK_NULL_HANDLE;
    texture.textureImageMemory = VK_NULL_HANDLE;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 6;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    MV_VULKAN(vkCreateImage(graphics.logicalDevice, &imageInfo, nullptr, &texture.textureImage));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(graphics.logicalDevice, texture.textureImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(graphics.physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    MV_VULKAN(vkAllocateMemory(graphics.logicalDevice, &allocInfo, nullptr, &texture.textureImageMemory));
    MV_VULKAN(vkBindImageMemory(graphics.logicalDevice, texture.textureImage, texture.textureImageMemory, 0));

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = imageInfo.mipLevels;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 6;

    VkCommandBuffer commandBuffer = begin_command_buffer(graphics);
    transition_image_layout(commandBuffer, texture.textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);
    submit_command_buffer(graphics, commandBuffer);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV = samplerInfo.addressModeU;
    samplerInfo.addressModeW = samplerInfo.addressModeU;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    MV_VULKAN(vkCreateSampler(graphics.logicalDevice, &samplerInfo, nullptr, &texture.imageInfo.sampler));

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture.textureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_R };
    viewInfo.format = format;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 6;
    viewInfo.subresourceRange.aspectMask = aspect;
    MV_VULKAN(vkCreateImageView(graphics.logicalDevice, &viewInfo, nullptr, &texture.imageInfo.imageView));

    texture.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    texture.imguiID = ImGui_ImplVulkan_AddTexture(texture.imageInfo.sampler, texture.imageInfo.imageView, texture.imageInfo.imageLayout);
    return texture;
}

mvTextureManager
DearPy3D::create_texture_manager()
{
    mvTextureManager manager;
    manager.textureKeys = nullptr;
    manager.maxTextureCount = 500u;
    manager.textureCount = 0u;
    manager.textures = nullptr;

    manager.textures = new mvTexture[manager.maxTextureCount];
    manager.textureKeys = new std::string[manager.maxTextureCount];

    for (u32 i = 0; i < manager.maxTextureCount; i++)
    {
        manager.textureKeys[i] = "";
    }

    return manager;
}

void
DearPy3D::cleanup_texture_manager(mvGraphics& graphics, mvTextureManager& manager)
{
    for (int i = 0; i < manager.textureCount; i++)
    {
        vkDestroySampler(graphics.logicalDevice, manager.textures[i].imageInfo.sampler, nullptr);
        vkDestroyImageView(graphics.logicalDevice, manager.textures[i].imageInfo.imageView, nullptr);
        vkDestroyImage(graphics.logicalDevice, manager.textures[i].textureImage, nullptr);
        vkFreeMemory(graphics.logicalDevice, manager.textures[i].textureImageMemory, nullptr);
    }

    manager.textureCount = 0u;
    delete[] manager.textures;
    delete[] manager.textureKeys;
}

mvAssetID
DearPy3D::register_texture(mvTextureManager& manager, const std::string& tag, mvTexture texture)
{
    assert(manager.textureCount <= manager.maxTextureCount);
    manager.textures[manager.textureCount] = texture;
    manager.textureKeys[manager.textureCount] = tag;
    manager.textureCount++;
    return manager.textureCount - 1;
}

mvAssetID
DearPy3D::register_texture_safe_load(mvGraphics& graphics, mvTextureManager& manager, const std::string& tag)
{
    assert(manager.textureCount <= manager.maxTextureCount);

    // check if texture is already loaded
    for (int i = 0; i < manager.textureCount; i++)
    {
        if (manager.textureKeys[i] == tag)
            return i;
    }

    return register_texture(manager, tag, create_texture(graphics, tag.c_str()));
}

mvAssetID
DearPy3D::register_texture_cube_safe_load(mvGraphics& graphics, mvTextureManager& manager, const std::string& tag)
{
    assert(manager.textureCount <= manager.maxTextureCount);

    // check if texture is already loaded
    for (int i = 0; i < manager.textureCount; i++)
    {
        if (manager.textureKeys[i] == tag)
            return i;
    }

    return register_texture(manager, tag, create_texture_cube(graphics, tag.c_str()));
}