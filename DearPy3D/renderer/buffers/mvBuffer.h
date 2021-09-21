#pragma once

#include <vector>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include "mvContext.h"

namespace DearPy3D {

	struct mvBuffer
	{
		VkBuffer       buffer;
		VkDeviceMemory bufferMemory;
	};

	template <typename T>
	mvBuffer mvCreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		mvBuffer buffer;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(T);
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(mvGetLogicalDevice(), &bufferInfo, nullptr, &buffer.buffer) != VK_SUCCESS)
			throw std::runtime_error("failed to create buffer!");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(mvGetLogicalDevice(), buffer.buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = mvFindMemoryType(mvGetPhysicalDevice(), memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(mvGetLogicalDevice(), &allocInfo, nullptr, &buffer.bufferMemory) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate buffer memory!");

		vkBindBufferMemory(mvGetLogicalDevice(), buffer.buffer, buffer.bufferMemory, 0);

		return buffer;
	}

	template <typename T>
	void mvUpdateBuffer(mvBuffer& buffer, const T& data)
	{
		void* udata;
		vkMapMemory(mvGetLogicalDevice(), buffer.bufferMemory, 0, sizeof(T), 0, &udata);
		memcpy(udata, &data, sizeof(T));
		vkUnmapMemory(mvGetLogicalDevice(), buffer.bufferMemory);
	}

	void mvCleanupBuffer(mvBuffer& buffer)
	{
		vkDestroyBuffer(mvGetLogicalDevice(), buffer.buffer, nullptr);
		vkFreeMemory(mvGetLogicalDevice(), buffer.bufferMemory, nullptr);
	}
	
}