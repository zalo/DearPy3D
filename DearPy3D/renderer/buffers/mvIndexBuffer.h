#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "mvAllocator.h"

namespace DearPy3D {

	//-----------------------------------------------------------------------------
	// forward declarations
	//-----------------------------------------------------------------------------
	class mvGraphics;

	//-----------------------------------------------------------------------------
	// mvIndexBuffer
	//-----------------------------------------------------------------------------
	class mvIndexBuffer
	{

	public:

		mvIndexBuffer(const std::vector<uint16_t>& ibuf);
		void cleanup();
		void bind();
		uint32_t getVertexCount();

	private:

		std::vector<uint16_t> _indices;
		VkBuffer _indexBuffer;
		VmaAllocation _memoryAllocation;

	};

}