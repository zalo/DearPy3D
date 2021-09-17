#pragma once

#include <vector>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include "mvContext.h"

namespace DearPy3D {
	
	//---------------------------------------------------------------------
	// mvBuffer
	//---------------------------------------------------------------------
	template <typename T>
	class mvBuffer
	{

	public:

		mvBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
		{

			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = sizeof(T);
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(mvGetLogicalDevice(), &bufferInfo, nullptr, &_buffer) != VK_SUCCESS)
				throw std::runtime_error("failed to create buffer!");

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(mvGetLogicalDevice(), _buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = mvFindMemoryType(mvGetPhysicalDevice(), memRequirements.memoryTypeBits, properties);

			if (vkAllocateMemory(mvGetLogicalDevice(), &allocInfo, nullptr, &_bufferMemory) != VK_SUCCESS)
				throw std::runtime_error("failed to allocate buffer memory!");

			vkBindBufferMemory(mvGetLogicalDevice(), _buffer, _bufferMemory, 0);

		}

		void update(const T& data)
		{
			void* udata;
			vkMapMemory(mvGetLogicalDevice(), _bufferMemory, 0, sizeof(T), 0, &udata);
			memcpy(udata, &data, sizeof(T));
			vkUnmapMemory(mvGetLogicalDevice(), _bufferMemory);
		}

		void cleanup()
		{
			vkDestroyBuffer(mvGetLogicalDevice(), _buffer, nullptr);
			vkFreeMemory(mvGetLogicalDevice(), _bufferMemory, nullptr);
		}

		VkBuffer getBuffer() { return _buffer; }

	private:

		VkBuffer       _buffer;
		VkDeviceMemory _bufferMemory;

	};

}