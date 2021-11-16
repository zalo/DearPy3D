#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "mvTypes.h"

struct mvAssetManager;

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

mvDescriptorSetLayout mvCreateDescriptorSetLayout(std::vector<mvDescriptorSpec> specs);

// descriptors
mvDescriptor mvCreateUniformBufferDescriptor       (mvAssetManager& am, mvDescriptorSpec spec, const std::string& name, u64 size, void* data);
mvDescriptor mvCreateDynamicUniformBufferDescriptor(mvAssetManager& am, mvDescriptorSpec spec, const std::string& name, u64 count, u64 size, void* data);
mvDescriptor mvCreateTextureDescriptor             (mvAssetManager& am, mvDescriptorSpec spec);

// descriptor specs
mvDescriptorSpec mvCreateUniformBufferDescriptorSpec(u32 binding);
mvDescriptorSpec mvCreateDynamicUniformBufferDescriptorSpec(u32 binding);
mvDescriptorSpec mvCreateTextureDescriptorSpec      (u32 binding);

mvDescriptorSet mvCreateDescriptorSet(mvAssetManager& am, VkDescriptorSetLayout layout, mvAssetID pipelineLayout);
void            mvBindDescriptorSet  (mvAssetManager& am, mvDescriptorSet& descriptorSet, u32 set, u32 dynamicDescriptorCount = 0u, u32* dynamicOffset = nullptr);