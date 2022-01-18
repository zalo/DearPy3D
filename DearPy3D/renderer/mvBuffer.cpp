#include "mvBuffer.h"
#include <stdexcept>
#include "mvGraphics.h"

mv_internal mvBuffer 
i_create_buffer(mvGraphics& graphics, mvBufferSpecification specification)
{
    mvBuffer buffer{};
    buffer.specification = specification;
    
    if (specification.aligned)
    {
        buffer.actualSize = get_required_uniform_buffer_size(graphics, specification.size * specification.components) * specification.count;
        buffer.bufferInfo.range = get_required_uniform_buffer_size(graphics, specification.size * specification.components);
    }
    else
        buffer.actualSize = specification.size * specification.count * specification.components;

    // create buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = buffer.actualSize;
    bufferInfo.usage = specification.usageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    MV_VULKAN(vkCreateBuffer(graphics.logicalDevice, &bufferInfo, nullptr, &buffer.buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(graphics.logicalDevice, buffer.buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(graphics.physicalDevice, memRequirements.memoryTypeBits, specification.propertyFlags);

    MV_VULKAN(vkAllocateMemory(graphics.logicalDevice, &allocInfo, nullptr, &buffer.deviceMemory));

    MV_VULKAN(vkBindBufferMemory(graphics.logicalDevice, buffer.buffer, buffer.deviceMemory, 0));

    return buffer;
}

mvBuffer 
create_buffer(mvGraphics& graphics, mvBufferSpecification specification, void* data)
{
    if (data)
    {
        // create staging buffer
        mvBufferSpecification stagingSpec = specification;
        stagingSpec.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingSpec.propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        mvBuffer stagingBuffer = i_create_buffer(graphics, stagingSpec);

        if (specification.aligned) // add spacing
        {
            void* mapping;
            MV_VULKAN(vkMapMemory(graphics.logicalDevice, stagingBuffer.deviceMemory, 0, stagingBuffer.actualSize, 0, &mapping));
            char* mdata = (char*)mapping;
            char* sdata = (char*)data;

            for (size_t i = 0; i < specification.count; i++)
            {
                char* dst = &mdata[i * stagingBuffer.actualSize/specification.count];
                char* src = &sdata[i * specification.size * specification.components];
                memcpy(dst, src, specification.size * specification.components);
            }

            //memcpy(mapping, data, (size_t)specification.stride);
            vkUnmapMemory(graphics.logicalDevice, stagingBuffer.deviceMemory);
        }
        else
        {
            void* mapping;
            MV_VULKAN(vkMapMemory(graphics.logicalDevice, stagingBuffer.deviceMemory, 0, stagingBuffer.actualSize, 0, &mapping));
            memcpy(mapping, data, specification.size * specification.count * specification.components);
            vkUnmapMemory(graphics.logicalDevice, stagingBuffer.deviceMemory);
        }

        // create final buffer
        specification.usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        mvBuffer finalBuffer = i_create_buffer(graphics, specification);

        copy_buffer(graphics, stagingBuffer, finalBuffer);

        // cleanup staging buffer
        vkDestroyBuffer(graphics.logicalDevice, stagingBuffer.buffer, nullptr);
        vkFreeMemory(graphics.logicalDevice, stagingBuffer.deviceMemory, nullptr);

        finalBuffer.bufferInfo.buffer = finalBuffer.buffer; // descriptor info

        return finalBuffer;

    }
    return i_create_buffer(graphics, specification);
}

void
copy_buffer(mvGraphics& graphics, mvBuffer srcBuffer, mvBuffer dstBuffer)
{
    VkCommandBuffer commandBuffer = begin_command_buffer(graphics);

    VkBufferCopy copyRegion{};
    copyRegion.size = srcBuffer.actualSize;
    vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);

    submit_command_buffer(graphics, commandBuffer);
}

void
copy_buffer_to_image(mvGraphics& graphics, mvBuffer srcBuffer, VkImage dstImage, u32 width, u32 height, u32 layers)
{
    VkCommandBuffer commandBuffer = begin_command_buffer(graphics);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layers;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        srcBuffer.buffer,
        dstImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    submit_command_buffer(graphics, commandBuffer);
}

void 
update_buffer(mvGraphics& graphics, mvBuffer& buffer, void* data)
{
    void* mapping;
    MV_VULKAN(vkMapMemory(graphics.logicalDevice, buffer.deviceMemory, 0, buffer.actualSize, 0, &mapping));
    memcpy(mapping, data, (size_t)buffer.actualSize);
    vkUnmapMemory(graphics.logicalDevice, buffer.deviceMemory);
}

void
partial_buffer_update(mvGraphics& graphics, mvBuffer& buffer, void* data, u64 index)
{
    u64 individualBlock = buffer.bufferInfo.range;
    u64 offset = index * individualBlock;
    void* mapping;
    MV_VULKAN(vkMapMemory(graphics.logicalDevice, buffer.deviceMemory, 0, buffer.actualSize, 0, &mapping));
    char* mdata = (char*)mapping;
    memcpy((void*)&mdata[offset], data, individualBlock);
    vkUnmapMemory(graphics.logicalDevice, buffer.deviceMemory);
}
