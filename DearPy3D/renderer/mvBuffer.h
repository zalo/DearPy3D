#pragma once

#include <vector>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include "mvContext.h"
#include "mvTypes.h"

struct mvBuffer
{
    VkBuffer      buffer = VK_NULL_HANDLE;
    VmaAllocation memoryAllocation = VK_NULL_HANDLE;
    u32           count = 0u;
    u32           size = 0u;
};

mvBuffer mvCreateBuffer        (void* data, u64 count, u64 size, VkBufferUsageFlags flags);
mvBuffer mvCreateDynamicBuffer (void* data, u64 count, u64 size, VkBufferUsageFlags flags);
void     mvUpdateBuffer        (mvBuffer& buffer, void* data);
