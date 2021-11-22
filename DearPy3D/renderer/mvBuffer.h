#pragma once

#include <stdexcept>
#include <vulkan/vulkan.h>
#include "mvContext.h"
#include "mvTypes.h"

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

mvBuffer mvCreateBuffer     (mvBufferSpecification specification, void* data = nullptr);
void     mvCopyBuffer       (mvBuffer srcBuffer, mvBuffer dstBuffer);
void     mvCopyBufferToImage(mvBuffer srcBuffer, VkImage dstImage, u32 width, u32 height);

void     mvUpdateBuffer        (mvBuffer& buffer, void* data);
void     mvPartialUpdateBuffer (mvBuffer& buffer, void* data, u64 index);