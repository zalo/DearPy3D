#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace DearPy3D {

	class mvGraphics;
	class mvPhysicalDevice;

	class mvLogicalDevice
	{

	public:

		// resource utilities
		void        createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void        copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void        transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void        copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		VkCommandBuffer beginSingleTimeCommands();
		void            endSingleTimeCommands(VkCommandBuffer commandBuffer);

	};

}