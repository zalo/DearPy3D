#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "mvTypes.h"

struct mvAssetManager;

struct mvDescriptorSet
{
    VkDescriptorSet* descriptorSets;
    mvAssetID        pipelineLayout;
};

mvDescriptorSet mvCreateDescriptorSet(mvAssetManager& am, VkDescriptorSetLayout layout, mvAssetID pipelineLayout);
void            mvBindDescriptorSet  (mvAssetManager& am, mvDescriptorSet& descriptorSet, u32 set);