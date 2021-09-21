#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "mvVertexLayout.h"
#include "mvAllocator.h"

namespace DearPy3D {

	struct mvVertexBuffer
	{
		std::vector<float> vertices;
		VkBuffer           buffer = VK_NULL_HANDLE;
		VmaAllocation      memoryAllocation = VK_NULL_HANDLE;
	};

	mvVertexBuffer mvCreateVertexBuffer(const std::vector<float>& ibuf);
	void           mvCleanupVertexBuffer(mvVertexBuffer& buffer);

}