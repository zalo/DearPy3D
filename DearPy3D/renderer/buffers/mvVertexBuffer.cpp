#include "mvVertexBuffer.h"
#include <stdexcept>
#include "mvContext.h"

namespace DearPy3D {

    mvVertexBuffer mvCreateVertexBuffer(const std::vector<float>& vbuf)
	{

        mvVertexBuffer buffer{};
        buffer.vertices = vbuf;

        VkDeviceSize bufferSize = sizeof(float) * buffer.vertices.size();

        auto allocator = mvAllocator();

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkBuffer stagingBuffer;

        VmaAllocation stagingBufferAllocation = allocator.allocateBuffer(bufferInfo,
            VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);

        float* data = allocator.mapMemory<float>(stagingBufferAllocation);
        memcpy(data, buffer.vertices.data(), (size_t)bufferSize);
        allocator.unmapMemory(stagingBufferAllocation);

        VkBufferCreateInfo indexBufferInfo{};
        indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        indexBufferInfo.size = bufferSize;
        indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

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

        //VkDebugUtilsObjectNameInfoEXT info = {};
        //info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
        //info.pObjectName = "vertex buffer man";
        //vkSetDebugUtilsObjectNameEXT(graphics.getDevice(), &info);

        return buffer;
	}

    void mvCleanupVertexBuffer(mvVertexBuffer& buffer)
    {
        auto allocator = mvAllocator();
        vkDestroyBuffer(mvGetLogicalDevice(), buffer.buffer, nullptr);
        allocator.free(buffer.memoryAllocation);
    }
}