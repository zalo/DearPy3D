#include "mvMaterialBuffer.h"
#include <stdexcept>
#include "mvContext.h"
#include "mvDrawable.h"

namespace DearPy3D {

	mvMaterialBuffer mvCreateMaterialBuffer()
	{

		mvMaterialBuffer material{};

		material.buffer.resize(3);
		material.bufferMemory.resize(3);
		for (int i = 0; i < 3; i++)
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = 2 * mvGetRequiredUniformBufferSize(sizeof(mvMaterialData));
			bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(mvGetLogicalDevice(), &bufferInfo, nullptr, &material.buffer[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create buffer!");

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(mvGetLogicalDevice(), material.buffer[i], &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = mvFindMemoryType(mvGetPhysicalDevice(), 
				memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			if (vkAllocateMemory(mvGetLogicalDevice(), &allocInfo, nullptr, &material.bufferMemory[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to allocate buffer memory!");

			vkBindBufferMemory(mvGetLogicalDevice(), material.buffer[i], material.bufferMemory[i], 0);
		}

		return material;
	}

	void mvBind(mvMaterialBuffer& material, mvMaterialData data, uint32_t index)
	{
		size_t offset_size = mvGetRequiredUniformBufferSize(sizeof(mvMaterialData));
		auto cindex = GContext->graphics.currentImageIndex;
		void* udata;
		vkMapMemory(mvGetLogicalDevice(), material.bufferMemory[cindex], offset_size * index, offset_size, 0, &udata);
		memcpy(udata, &data, sizeof(mvMaterialData));
		vkUnmapMemory(mvGetLogicalDevice(), material.bufferMemory[cindex]);
	}

	void mvCleanupMaterialBuffer(mvMaterialBuffer& material)
	{
		for (int i = 0; i < 3; i++)
		{
			vkDestroyBuffer(mvGetLogicalDevice(), material.buffer[i], nullptr);
			vkFreeMemory(mvGetLogicalDevice(), material.bufferMemory[i], nullptr);
		}
	}
}