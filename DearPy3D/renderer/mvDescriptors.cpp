#include "mvDescriptors.h"
#include "mvContext.h"
#include "mvAssetManager.h"

mvDescriptorSetLayout
mvCreateDescriptorSetLayout(std::vector<mvDescriptorSpec> specs)
{
    mvDescriptorSetLayout layout{};
    layout.specs = specs;

    VkDescriptorSetLayoutBinding* bindings = new VkDescriptorSetLayoutBinding[specs.size()];

    for (u32 i = 0; i < specs.size(); i++)
        bindings[i] = specs[i].layoutBinding;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = specs.size();
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(mvGetLogicalDevice(), &layoutInfo, nullptr, &layout.layout) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout!");

    delete[] bindings;


    return layout;
}

mvDescriptor
mvCreateUniformBufferDescriptor(mvAssetManager& am, mvDescriptorSpec spec, const std::string& name, u64 size, void* data)
{
    mvDescriptor descriptor{};
    descriptor.size = size;
    descriptor.spec = spec;

    // VkWriteDescriptorSet
    descriptor.write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor.write.dstBinding = spec.layoutBinding.binding;
    descriptor.write.dstArrayElement = 0;
    descriptor.write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor.write.descriptorCount = 1;

    // handled during update
    //descriptor.write.dstSet = scene.descriptorSets.descriptorSets[i];
    //descriptor.write.pBufferInfo = &am.buffers[scene.buffers[i]].asset.bufferInfo;
    

    for (size_t i = 0; i < GContext->graphics.swapChainImages.size(); i++)
        descriptor.bufferID.push_back(mvRegisterAsset(&am, name+std::to_string(i),
            mvCreateDynamicBuffer(data, 1, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)));

    return descriptor;
}

mvDescriptor
mvCreateDynamicUniformBufferDescriptor(mvAssetManager& am, mvDescriptorSpec spec, const std::string& name, u64 count, u64 size, void* data)
{
    mvDescriptor descriptor{};
    descriptor.size = size;
    descriptor.spec = spec;

    // VkWriteDescriptorSet
    descriptor.write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor.write.dstBinding = spec.layoutBinding.binding;
    descriptor.write.dstArrayElement = 0;
    descriptor.write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descriptor.write.descriptorCount = 1;

    // handled during update
    //descriptor.write.dstSet = scene.descriptorSets.descriptorSets[i];
    //descriptor.write.pBufferInfo = &am.buffers[scene.buffers[i]].asset.bufferInfo;


    for (size_t i = 0; i < GContext->graphics.swapChainImages.size(); i++)
        descriptor.bufferID.push_back(mvRegisterAsset(&am, name + std::to_string(i),
            mvCreateDynamicBuffer(data, count, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)));

    return descriptor;
}

mvDescriptor
mvCreateTextureDescriptor(mvAssetManager& am, mvDescriptorSpec spec)
{
    mvDescriptor descriptor{};
    descriptor.size = 0u;
    descriptor.spec = spec;

    // VkWriteDescriptorSet
    descriptor.write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor.write.dstBinding = spec.layoutBinding.binding;
    descriptor.write.dstArrayElement = 0;
    descriptor.write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor.write.descriptorCount = 1;
    
    // handled during update
    //descriptor.write.dstSet = scene.descriptorSets.descriptorSets[i];
    //descriptor.write.pImageInfo = &am.textures[textureID].asset.imageInfo;

    return descriptor;
}

VkDescriptorSetLayout
mvCreateDescriptorSetLayout(mvDescriptorSetLayout descriptorSetLayout)
{

    VkDescriptorSetLayoutBinding* bindings = new VkDescriptorSetLayoutBinding[descriptorSetLayout.specs.size()];

    for(u32 i = 0; i < descriptorSetLayout.specs.size(); i++)
        bindings[i] = descriptorSetLayout.specs[i].layoutBinding;

    VkDescriptorSetLayout layout = VK_NULL_HANDLE;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = descriptorSetLayout.specs.size();
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
mvCreateDynamicUniformBufferDescriptorSpec(u32 binding)
{
    mvDescriptorSpec descriptor{};
    descriptor.layoutBinding.binding = binding;
    descriptor.layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
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

    // allocate descriptor sets
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