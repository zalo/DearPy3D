#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "mvDearPy3D.h"

namespace DearPy3D
{
    // descriptors
    mvDescriptor create_uniform_descriptor        (mvGraphics& graphics, mvDescriptorSpec spec, const std::string& name, size_t size, void* data);
    mvDescriptor create_dynamic_uniform_descriptor(mvGraphics& graphics, mvDescriptorSpec spec, const std::string& name, size_t count, size_t size, void* data);
    mvDescriptor create_texture_descriptor        (mvDescriptorSpec spec);

    // descriptor specs
    mvDescriptorSpec create_uniform_descriptor_spec        (unsigned binding);
    mvDescriptorSpec create_dynamic_uniform_descriptor_spec(unsigned binding);
    mvDescriptorSpec create_texture_descriptor_spec        (unsigned binding);

    // descriptor sets
    mvDescriptorSet       create_descriptor_set       (mvGraphics& graphics, VkDescriptorSetLayout layout, VkPipelineLayout pipelineLayout);
    void                  bind_descriptor_set         (mvGraphics& graphics, mvDescriptorSet& descriptorSet, unsigned set, unsigned dynamicDescriptorCount = 0u, unsigned* dynamicOffset = nullptr);
    mvDescriptorSetLayout create_descriptor_set_layout(mvGraphics& graphics, std::vector<mvDescriptorSpec> specs);

    mvDescriptorManager   create_descriptor_manager();
    void                  cleanup_descriptor_manager    (mvGraphics& graphics, mvDescriptorManager& manager);
    mvAssetID             register_descriptor_set       (mvDescriptorManager& manager, const std::string& tag, mvDescriptorSet pipeline);
    mvAssetID             register_descriptor_set_layout(mvDescriptorManager& manager, const std::string& tag, VkDescriptorSetLayout layout);
    VkDescriptorSet       get_descriptor_set            (mvDescriptorManager& manager, const std::string& tag, unsigned index);
    VkDescriptorSetLayout get_descriptor_set_layout     (mvDescriptorManager& manager, const std::string& tag);
}

struct mvDescriptorSpec
{
    VkDescriptorSetLayoutBinding layoutBinding;
};

struct mvDescriptorSetLayout
{
    std::vector<mvDescriptorSpec> specs;
    VkDescriptorSetLayout         layout;
};

struct mvDescriptor
{
    std::vector<mvBuffer>  buffers;
    VkDeviceSize           size;
    mvDescriptorSpec       spec;
    VkWriteDescriptorSet   write;
};

struct mvDescriptorSet
{
    VkDescriptorSet*          descriptorSets;
    VkPipelineLayout          pipelineLayout;
    std::vector<mvDescriptor> descriptors;
};

struct mvDescriptorManager
{
    std::string*     descriptorSetKeys;
    std::string*     layoutKeys;
    unsigned         maxDSCount;
    unsigned         dsCount;
    unsigned         maxLayoutCount;
    unsigned         layoutCount;
    mvDescriptorSet* descriptorSets;
    VkDescriptorSetLayout* layouts;
};