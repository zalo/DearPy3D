#pragma once

#include <stdexcept>
#include <vulkan/vulkan.h>
#include "mvTypes.h"

// forward declarations
struct mvGraphics;
struct mvBufferSpecification;
struct mvBuffer;

mvBuffer create_buffer        (mvGraphics& graphics, mvBufferSpecification specification, void* data = nullptr);
void     copy_buffer          (mvGraphics& graphics, mvBuffer srcBuffer, mvBuffer dstBuffer);
void     copy_buffer_to_image (mvGraphics& graphics, mvBuffer srcBuffer, VkImage dstImage, u32 width, u32 height, u32 layers = 1u);
void     update_buffer        (mvGraphics& graphics, mvBuffer& buffer, void* data);
void     partial_buffer_update(mvGraphics& graphics, mvBuffer& buffer, void* data, u64 index);

struct mvBufferSpecification
{
    u64                   size;             // component size
    u64                   count = 1u;       // item count
    u64                   components = 1u;  // component count
    b8                    aligned = false;  // use for UBOs
    VkBufferUsageFlags    usageFlags;
    VkMemoryPropertyFlags propertyFlags;
};

struct mvBuffer
{
    VkBuffer               buffer = VK_NULL_HANDLE;
    VkDeviceMemory         deviceMemory = VK_NULL_HANDLE;
    VkDeviceSize           actualSize = 0u;
    VkDescriptorBufferInfo bufferInfo;
    mvBufferSpecification  specification;
};

