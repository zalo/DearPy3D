#include "mvDescriptorSet.h"
#include "mvContext.h"
#include "mvAssetManager.h"

mvDescriptorSet
mvCreateDescriptorSet(mvAssetManager& am, VkDescriptorSetLayout layout, mvAssetID pipelineLayout)
{
    mvDescriptorSet descriptorSet{};
    descriptorSet.pipelineLayout = pipelineLayout;
    descriptorSet.descriptorSets = new VkDescriptorSet[GContext->graphics.swapChainImages.size()];

    //-----------------------------------------------------------------------------
    // allocate descriptor sets
    //-----------------------------------------------------------------------------
    std::vector<VkDescriptorSetLayout> layouts(GContext->graphics.swapChainImages.size(), layout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = GContext->graphics.descriptorPool;
    allocInfo.descriptorSetCount = GContext->graphics.swapChainImages.size();
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(mvGetLogicalDevice(), &allocInfo, descriptorSet.descriptorSets) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate descriptor sets!");

    return descriptorSet;
}

void
mvBindDescriptorSet(mvAssetManager& am, mvDescriptorSet& descriptorSet, u32 set)
{
    VkDescriptorSet rawDescriptorSet = descriptorSet.descriptorSets[GContext->graphics.currentImageIndex];
    VkPipelineLayout layout = am.pipelineLayouts[descriptorSet.pipelineLayout].asset;
    vkCmdBindDescriptorSets(mvGetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, set, 1, &rawDescriptorSet, 0, nullptr);
}