#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "mvTypes.h"

// forward declarations
struct mvAssetManager;
struct mvGraphics;
struct mvDescriptorSpec;
struct mvDescriptorSetLayout;
struct mvDescriptor;
struct mvDescriptorSet;

mvDescriptorSetLayout create_descriptor_set_layout(mvGraphics& graphics, std::vector<mvDescriptorSpec> specs);

// descriptors
mvDescriptor create_uniform_descriptor        (mvGraphics& graphics, mvAssetManager& am, mvDescriptorSpec spec, const std::string& name, u64 size, void* data);
mvDescriptor create_dynamic_uniform_descriptor(mvGraphics& graphics, mvAssetManager& am, mvDescriptorSpec spec, const std::string& name, u64 count, u64 size, void* data);
mvDescriptor create_texture_descriptor        (mvAssetManager& am, mvDescriptorSpec spec);

// descriptor specs
mvDescriptorSpec create_uniform_descriptor_spec(u32 binding);
mvDescriptorSpec create_dynamic_uniform_descriptor_spec(u32 binding);
mvDescriptorSpec create_texture_descriptor_spec      (u32 binding);

mvDescriptorSet create_descriptor_set(mvGraphics& graphics, mvAssetManager& am, VkDescriptorSetLayout layout, mvAssetID pipelineLayout);
void            bind_descriptor_set  (mvGraphics& graphics, mvAssetManager& am, mvDescriptorSet& descriptorSet, u32 set, u32 dynamicDescriptorCount = 0u, u32* dynamicOffset = nullptr);

struct mvDescriptorSpec
{
    VkDescriptorSetLayoutBinding layoutBinding;
};

struct mvDescriptorSetLayout
{
    std::vector<mvDescriptorSpec> specs;
    VkDescriptorSetLayout         layout = VK_NULL_HANDLE;
};

struct mvDescriptor
{
    std::vector<mvAssetID> bufferID;
    VkDeviceSize           size = 0u;
    mvDescriptorSpec       spec;
    VkWriteDescriptorSet   write;
};

struct mvDescriptorSet
{
    VkDescriptorSet*          descriptorSets;
    mvAssetID                 pipelineLayout;
    std::vector<mvDescriptor> descriptors;
};