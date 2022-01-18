#include "mvDescriptors.h"
#include "mvAssetManager.h"
#include "mvGraphics.h"

mvDescriptorSetLayout
create_descriptor_set_layout(mvGraphics& graphics, std::vector<mvDescriptorSpec> specs)
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

    MV_VULKAN(vkCreateDescriptorSetLayout(graphics.logicalDevice, &layoutInfo, nullptr, &layout.layout));
    delete[] bindings;
    return layout;
}

mvDescriptor
create_uniform_descriptor(mvGraphics& graphics, mvAssetManager& am, mvDescriptorSpec spec, const std::string& name, u64 size, void* data)
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
    
    mvBufferSpecification bufferSpec{};
    bufferSpec.aligned = true;
    bufferSpec.count = 1;
    bufferSpec.size = size;
    bufferSpec.components = 1;
    bufferSpec.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferSpec.propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    for (size_t i = 0; i < graphics.swapChainImages.size(); i++)
        descriptor.bufferID.push_back(mvRegisterAsset(&am, name+std::to_string(i), create_buffer(graphics, bufferSpec, data)));

    return descriptor;
}

mvDescriptor
create_dynamic_uniform_descriptor(mvGraphics& graphics, mvAssetManager& am, mvDescriptorSpec spec, const std::string& name, u64 count, u64 size, void* data)
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

    mvBufferSpecification bufferSpec{};
    bufferSpec.aligned = true;
    bufferSpec.count = count;
    bufferSpec.size = size;
    bufferSpec.components = 1;
    bufferSpec.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferSpec.propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    for (size_t i = 0; i < graphics.swapChainImages.size(); i++)
        descriptor.bufferID.push_back(mvRegisterAsset(&am, name + std::to_string(i), create_buffer(graphics, bufferSpec, data)));

    return descriptor;
}

mvDescriptor
create_texture_descriptor(mvAssetManager& am, mvDescriptorSpec spec)
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

mvDescriptorSpec
create_uniform_descriptor_spec(u32 binding)
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
create_dynamic_uniform_descriptor_spec(u32 binding)
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
create_texture_descriptor_spec(u32 binding)
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
create_descriptor_set(mvGraphics& graphics, mvAssetManager& am, VkDescriptorSetLayout layout, mvAssetID pipelineLayout)
{
    mvDescriptorSet descriptorSet{};
    descriptorSet.pipelineLayout = pipelineLayout;
    descriptorSet.descriptorSets = new VkDescriptorSet[graphics.swapChainImages.size()];

    // allocate descriptor sets
    std::vector<VkDescriptorSetLayout> layouts(graphics.swapChainImages.size(), layout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = graphics.descriptorPool;
    allocInfo.descriptorSetCount = graphics.swapChainImages.size();
    allocInfo.pSetLayouts = layouts.data();

    MV_VULKAN(vkAllocateDescriptorSets(graphics.logicalDevice, &allocInfo, descriptorSet.descriptorSets));

    return descriptorSet;
}

void
bind_descriptor_set(mvGraphics& graphics, mvAssetManager& am, mvDescriptorSet& descriptorSet, u32 set, u32 dynamicDescriptorCount, u32* dynamicOffset)
{
    VkDescriptorSet rawDescriptorSet = descriptorSet.descriptorSets[graphics.currentImageIndex];
    VkPipelineLayout layout = am.pipelineLayouts[descriptorSet.pipelineLayout].asset;
    vkCmdBindDescriptorSets(get_current_command_buffer(graphics), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, set, 1, &rawDescriptorSet, dynamicDescriptorCount, dynamicOffset);
}