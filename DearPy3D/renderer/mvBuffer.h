#pragma once

#include <stdexcept>
#include <vulkan/vulkan.h>
#include "mvContext.h"
#include "mvTypes.h"

struct mvBufferSpecification
{
    VkDeviceSize          size;
    VkBufferUsageFlags    usageFlags;
    VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    u64                   count = 1u;       // use for UBO
    u64                   components = 1u;      // use for UBO
    b8                    aligned = false;  // use for UBO
};

struct mvBuffer
{
    VkBuffer               buffer = VK_NULL_HANDLE;
    VkDeviceMemory         deviceMemory = VK_NULL_HANDLE;
    u32                    actualSize = 0u;
    VkDescriptorBufferInfo bufferInfo;
    mvBufferSpecification  specification;
};

mvBuffer mvCreateBuffer(mvBufferSpecification specification, void* data = nullptr);
void     mvCopyBuffer  (mvBuffer srcBuffer, mvBuffer dstBuffer);

void     mvUpdateBuffer        (mvBuffer& buffer, void* data);
void     mvPartialUpdateBuffer (mvBuffer& buffer, void* data, u64 index);