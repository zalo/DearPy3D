#include "mvIndexBuffer.h"
#include <stdexcept>
#include "mvContext.h"

namespace DearPy3D {

	mvIndexBuffer mvCreateIndexBuffer(const std::vector<uint16_t>& ibuf)
	{
        mvIndexBuffer buffer{};
        buffer.indices = ibuf;

        VkDeviceSize bufferSize = sizeof(buffer.indices[0]) * buffer.indices.size();

        auto allocator = mvAllocator();

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkBuffer stagingBuffer;

        VmaAllocation stagingBufferAllocation = allocator.allocateBuffer(bufferInfo,
            VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);

        uint16_t* data = allocator.mapMemory<uint16_t>(stagingBufferAllocation);
        memcpy(data, buffer.indices.data(), (size_t)bufferSize);
        allocator.unmapMemory(stagingBufferAllocation);

        VkBufferCreateInfo indexBufferInfo{};
        indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        indexBufferInfo.size = bufferSize;
        indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        buffer.memoryAllocation = allocator.allocateBuffer(indexBufferInfo, VMA_MEMORY_USAGE_GPU_ONLY, buffer.buffer);

        VkCommandBuffer copyCmd = mvBeginSingleTimeCommands();

        VkBufferCopy copyRegion = {};
        copyRegion.size = bufferSize;
        vkCmdCopyBuffer(
            copyCmd,
            stagingBuffer,
            buffer.buffer,
            1,
            &copyRegion);

        mvEndSingleTimeCommands(copyCmd);

        allocator.destroyBuffer(stagingBuffer, stagingBufferAllocation);

        return buffer;
	}

    void mvCleanupIndexBuffer(mvIndexBuffer& buffer)
    {
        auto allocator = mvAllocator();
        vkDestroyBuffer(mvGetLogicalDevice(), buffer.buffer, nullptr);
        allocator.free(buffer.memoryAllocation);
    }

}