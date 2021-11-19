#include "mvBuffer.h"
#include <stdexcept>
#include "mvContext.h"

mv_internal mvBuffer 
create_buffer(mvBufferSpecification specification)
{
    mvBuffer buffer{};
    buffer.specification = specification;
    
    if (specification.aligned)
    {
        buffer.actualSize = mvGetRequiredUniformBufferSize(specification.size * specification.components) * specification.count;
        buffer.bufferInfo.range = mvGetRequiredUniformBufferSize(specification.size * specification.components);
    }
    else
        buffer.actualSize = specification.size * specification.count * specification.components;

    // create buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = buffer.actualSize;
    bufferInfo.usage = specification.usageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    MV_VULKAN(vkCreateBuffer(mvGetLogicalDevice(), &bufferInfo, nullptr, &buffer.buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(mvGetLogicalDevice(), buffer.buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = mvFindMemoryType(GContext->graphics.physicalDevice, memRequirements.memoryTypeBits, specification.propertyFlags);

    MV_VULKAN(vkAllocateMemory(mvGetLogicalDevice(), &allocInfo, nullptr, &buffer.deviceMemory));

    MV_VULKAN(vkBindBufferMemory(mvGetLogicalDevice(), buffer.buffer, buffer.deviceMemory, 0));

    return buffer;
}

mvBuffer mvCreateBuffer(mvBufferSpecification specification, void* data)
{
    if (data)
    {
        // create staging buffer
        mvBufferSpecification stagingSpec = specification;
        stagingSpec.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingSpec.propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        mvBuffer stagingBuffer = create_buffer(stagingSpec);

        if (specification.aligned) // add spacing
        {
            void* mapping;
            MV_VULKAN(vkMapMemory(mvGetLogicalDevice(), stagingBuffer.deviceMemory, 0, stagingBuffer.actualSize, 0, &mapping));
            char* mdata = (char*)mapping;
            char* sdata = (char*)data;

            for (size_t i = 0; i < specification.count; i++)
            {
                char* dst = &mdata[i * stagingBuffer.actualSize/specification.count];
                char* src = &sdata[i * specification.size * specification.components];
                memcpy(dst, src, specification.size * specification.components);
            }

            //memcpy(mapping, data, (size_t)specification.stride);
            vkUnmapMemory(mvGetLogicalDevice(), stagingBuffer.deviceMemory);
        }
        else
        {
            void* mapping;
            MV_VULKAN(vkMapMemory(mvGetLogicalDevice(), stagingBuffer.deviceMemory, 0, stagingBuffer.actualSize, 0, &mapping));
            memcpy(mapping, data, (size_t)specification.size * specification.count * specification.components);
            vkUnmapMemory(mvGetLogicalDevice(), stagingBuffer.deviceMemory);
        }

        // create final buffer
        specification.usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        mvBuffer finalBuffer = create_buffer(specification);

        mvCopyBuffer(stagingBuffer, finalBuffer);

        // cleanup staging buffer
        vkDestroyBuffer(mvGetLogicalDevice(), stagingBuffer.buffer, nullptr);
        vkFreeMemory(mvGetLogicalDevice(), stagingBuffer.deviceMemory, nullptr);

        finalBuffer.bufferInfo.buffer = finalBuffer.buffer; // descriptor info

        return finalBuffer;

    }
    return create_buffer(specification);
}

void
mvCopyBuffer(mvBuffer srcBuffer, mvBuffer dstBuffer)
{
    VkCommandBuffer commandBuffer = mvBeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = srcBuffer.actualSize;
    vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);

    mvEndSingleTimeCommands(commandBuffer);
}

void 
mvUpdateBuffer(mvBuffer& buffer, void* data)
{
    void* mapping;
    MV_VULKAN(vkMapMemory(mvGetLogicalDevice(), buffer.deviceMemory, 0, buffer.actualSize, 0, &mapping));
    memcpy(mapping, data, (size_t)buffer.actualSize);
    vkUnmapMemory(mvGetLogicalDevice(), buffer.deviceMemory);
}

void
mvPartialUpdateBuffer(mvBuffer& buffer, void* data, u64 index)
{
    u64 individualBlock = buffer.bufferInfo.range;
    u64 offset = index * individualBlock;
    void* mapping;
    MV_VULKAN(vkMapMemory(mvGetLogicalDevice(), buffer.deviceMemory, 0, buffer.actualSize, 0, &mapping));
    char* mdata = (char*)mapping;
    memcpy((void*)&mdata[offset], data, individualBlock);
    vkUnmapMemory(mvGetLogicalDevice(), buffer.deviceMemory);
}
