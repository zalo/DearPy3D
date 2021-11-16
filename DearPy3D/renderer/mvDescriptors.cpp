#include "mvDescriptors.h"
#include "mvContext.h"
#include "mvAssetManager.h"

VkDescriptorSetLayout
mvCreateDescriptorSetLayout(mvDescriptorSpec* descriptors, u32 count)
{

    VkDescriptorSetLayoutBinding* bindings = new VkDescriptorSetLayoutBinding[count];

    for(u32 i = 0; i < count; i++)
        bindings[i] = descriptors[i].layoutBinding;

    VkDescriptorSetLayout layout = VK_NULL_HANDLE;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = count;
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(mvGetLogicalDevice(), &layoutInfo, nullptr, &layout) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout!");

    delete[] bindings;

    return layout;
}

mvDescriptorSpec
mvCreateUniformBufferDescriptorSpec(u32 binding)
{
    mvDescriptorSpec descriptor{};
    descriptor.layoutBinding.binding = binding;
    descriptor.layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor.layoutBinding.descriptorCount = 1;
    descriptor.layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptor.layoutBinding.pImmutableSamplers = nullptr;
    return descriptor;
}

mvDescriptorSpec
mvCreateTextureDescriptorSpec(u32 binding)
{
    mvDescriptorSpec descriptor{};
    descriptor.layoutBinding.binding = binding;
    descriptor.layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor.layoutBinding.descriptorCount = 1;
    descriptor.layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptor.layoutBinding.pImmutableSamplers = nullptr;
    return descriptor;
}

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
mvBindDescriptorSet(mvAssetManager& am, mvDescriptorSet& descriptorSet, u32 set, u32 dynamicDescriptorCount, u32* dynamicOffset)
{
    VkDescriptorSet rawDescriptorSet = descriptorSet.descriptorSets[GContext->graphics.currentImageIndex];
    VkPipelineLayout layout = am.pipelineLayouts[descriptorSet.pipelineLayout].asset;
    vkCmdBindDescriptorSets(mvGetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, set, 1, &rawDescriptorSet, dynamicDescriptorCount, dynamicOffset);
}