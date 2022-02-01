#pragma once

#include <vulkan/vulkan.h>
#include "mvDearPy3D.h"

namespace DearPy3D
{
    mvBuffer create_buffer        (mvGraphics& graphics, mvBufferSpecification specification, void* data = nullptr);
    void     copy_buffer          (mvGraphics& graphics, mvBuffer srcBuffer, mvBuffer dstBuffer);
    void     copy_buffer_to_image (mvGraphics& graphics, mvBuffer srcBuffer, VkImage dstImage, unsigned width, unsigned height, unsigned layers = 1u);
    void     update_buffer        (mvGraphics& graphics, mvBuffer& buffer, void* data);
    void     partial_buffer_update(mvGraphics& graphics, mvBuffer& buffer, void* data, size_t index);
}

struct mvBufferSpecification
{
    size_t                size;       // component size
    size_t                count;      // item count
    size_t                components; // component count
    bool                  aligned;    // use for UBOs
    VkBufferUsageFlags    usageFlags;
    VkMemoryPropertyFlags propertyFlags;
};

struct mvBuffer
{
    VkBuffer               buffer;
    VkDeviceMemory         deviceMemory;
    VkDeviceSize           actualSize;
    VkDescriptorBufferInfo bufferInfo;
    mvBufferSpecification  specification;
};
