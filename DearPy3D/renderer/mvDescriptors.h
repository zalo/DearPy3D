#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "mvTypes.h"

struct mvAssetManager;

struct mvDescriptorSpec
{
    VkDescriptorSetLayoutBinding layoutBinding;
};

struct mvDescriptorSet
{
    VkDescriptorSet* descriptorSets;
    mvAssetID        pipelineLayout;
};

VkDescriptorSetLayout mvCreateDescriptorSetLayout(mvDescriptorSpec* descriptors, u32 count);

mvDescriptorSpec mvCreateUniformBufferDescriptorSpec(u32 binding);
mvDescriptorSpec mvCreateTextureDescriptorSpec      (u32 binding);

mvDescriptorSet mvCreateDescriptorSet(mvAssetManager& am, VkDescriptorSetLayout layout, mvAssetID pipelineLayout);
void            mvBindDescriptorSet  (mvAssetManager& am, mvDescriptorSet& descriptorSet, u32 set, u32 dynamicDescriptorCount = 0u, u32* dynamicOffset = nullptr);