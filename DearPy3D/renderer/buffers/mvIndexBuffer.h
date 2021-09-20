#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "mvAllocator.h"

namespace DearPy3D {

	struct mvIndexBuffer
	{
		std::vector<uint16_t> indices;
		VkBuffer              buffer           = VK_NULL_HANDLE;
		VmaAllocation         memoryAllocation = VK_NULL_HANDLE;
	};

	mvIndexBuffer mvCreateIndexBuffer(const std::vector<uint16_t>& ibuf);
	void          mvCleanupIndexBuffer(mvIndexBuffer& buffer);

}