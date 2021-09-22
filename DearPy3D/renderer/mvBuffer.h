#pragma once

#include <vector>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include "mvContext.h"

namespace DearPy3D {

	struct mvBuffer
	{
		VkBuffer      buffer = VK_NULL_HANDLE;
		VmaAllocation memoryAllocation = VK_NULL_HANDLE;
		size_t        count = 0u;
	};

	mvBuffer mvCreateBuffer       (void* data, uint64_t count, uint64_t size, VkBufferUsageFlags flags);
	void     mvCleanupBuffer(mvBuffer& buffer);

}