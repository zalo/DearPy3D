#include "mvDescriptors.h"
#include "mvGraphics.h"
#include "mvBuffer.h"
#include <stdexcept>
#include <assert.h>

mvDescriptorSetLayout
DearPy3D::create_descriptor_set_layout(mvGraphics& graphics, std::vector<mvDescriptorSpec> specs)
{
    mvDescriptorSetLayout layout{};
    layout.specs = specs;
    layout.layout = VK_NULL_HANDLE;

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
DearPy3D::create_uniform_descriptor(mvGraphics& graphics, mvDescriptorSpec spec, const std::string& name, size_t size, void* data)
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
        descriptor.buffers.push_back(create_buffer(graphics, bufferSpec, data));

    return descriptor;
}

mvDescriptor
DearPy3D::create_dynamic_uniform_descriptor(mvGraphics& graphics, mvDescriptorSpec spec, const std::string& name, size_t count, size_t size, void* data)
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
        descriptor.buffers.push_back(create_buffer(graphics, bufferSpec, data));

    return descriptor;
}

mvDescriptor
DearPy3D::create_texture_descriptor(mvDescriptorSpec spec)
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
DearPy3D::create_uniform_descriptor_spec(unsigned binding)
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
DearPy3D::create_dynamic_uniform_descriptor_spec(unsigned binding)
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
DearPy3D::create_texture_descriptor_spec(unsigned binding)
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
DearPy3D::create_descriptor_set(mvGraphics& graphics, VkDescriptorSetLayout layout, VkPipelineLayout pipelineLayout)
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
DearPy3D::bind_descriptor_set(mvGraphics& graphics, mvDescriptorSet& descriptorSet, unsigned set, unsigned dynamicDescriptorCount, unsigned* dynamicOffset)
{
    VkDescriptorSet rawDescriptorSet = descriptorSet.descriptorSets[graphics.currentImageIndex];
    vkCmdBindDescriptorSets(get_current_command_buffer(graphics), VK_PIPELINE_BIND_POINT_GRAPHICS, descriptorSet.pipelineLayout, set, 1, &rawDescriptorSet, dynamicDescriptorCount, dynamicOffset);
}

mvDescriptorManager
DearPy3D::create_descriptor_manager()
{
    mvDescriptorManager manager{};
    manager.descriptorSetKeys = nullptr;
    manager.layoutKeys = nullptr;      	  
    manager.maxDSCount = 50u;
    manager.dsCount = 0u;
    manager.descriptorSets = nullptr;    	  
    manager.maxLayoutCount = 50u;
    manager.layoutCount = 0u;
    manager.layouts = nullptr;

    manager.descriptorSets = new mvDescriptorSet[manager.maxDSCount];
    manager.descriptorSetKeys = new std::string[manager.maxDSCount];
    manager.layouts = new VkDescriptorSetLayout[manager.maxLayoutCount];
    manager.layoutKeys = new std::string[manager.maxLayoutCount];

    for (u32 i = 0; i < manager.maxDSCount; i++)
    {
        manager.descriptorSetKeys[i] = "";
    }

    for (u32 i = 0; i < manager.maxLayoutCount; i++)
    {
        manager.layoutKeys[i] = "";
        manager.layouts[i] = VK_NULL_HANDLE;
    }

    return manager;
}

void
DearPy3D::cleanup_descriptor_manager(mvGraphics& graphics, mvDescriptorManager& manager)
{

    // cleanup descriptor sets
    for (int i = 0; i < manager.dsCount; i++)
    {
        mvDescriptorSet set = manager.descriptorSets[i];

        for (int j = 0; j < set.descriptors.size(); j++)
        {
            for (int k = 0; k < set.descriptors[j].buffers.size(); k++)
            {
                vkDestroyBuffer(graphics.logicalDevice, set.descriptors[j].buffers[k].buffer, nullptr);
                vkFreeMemory(graphics.logicalDevice, set.descriptors[j].buffers[k].deviceMemory, nullptr);
                set.descriptors[j].buffers[k].buffer = VK_NULL_HANDLE;
                set.descriptors[j].buffers[k].deviceMemory = VK_NULL_HANDLE;
            }
        }

        delete[] manager.descriptorSets[i].descriptorSets;
    }
    manager.dsCount = 0u;

    // descriptor set layouts
    for (int i = 0; i < manager.layoutCount; i++)
    {
        vkDestroyDescriptorSetLayout(graphics.logicalDevice, manager.layouts[i], nullptr);
    }

    manager.dsCount = 0u;
    manager.layoutCount = 0u;

    delete[] manager.descriptorSets;
    delete[] manager.descriptorSetKeys;
    delete[] manager.layouts;
    delete[] manager.layoutKeys;
}

mvAssetID
DearPy3D::register_descriptor_set(mvDescriptorManager& manager, const std::string& tag, mvDescriptorSet pipeline)
{
    assert(manager.dsCount <= manager.maxDSCount);
    manager.descriptorSets[manager.dsCount] = pipeline;
    manager.descriptorSetKeys[manager.dsCount] = tag;
    manager.dsCount++;
    return manager.dsCount - 1;
}

mvAssetID
DearPy3D::register_descriptor_set_layout(mvDescriptorManager& manager, const std::string& tag, VkDescriptorSetLayout layout)
{
    assert(manager.layoutCount <= manager.maxLayoutCount);
    manager.layouts[manager.layoutCount] = layout;
    manager.layoutKeys[manager.layoutCount] = tag;
    manager.layoutCount++;
    return manager.layoutCount - 1;
}

VkDescriptorSet
DearPy3D::get_descriptor_set(mvDescriptorManager& manager, const std::string& tag, unsigned index)
{
    for (int i = 0; i < manager.dsCount; i++)
    {
        if (manager.descriptorSetKeys[i] == tag)
            return manager.descriptorSets[i].descriptorSets[index];
    }
    return VK_NULL_HANDLE;
}

VkDescriptorSetLayout
DearPy3D::get_descriptor_set_layout(mvDescriptorManager& manager, const std::string& tag)
{
    for (int i = 0; i < manager.layoutCount; i++)
    {
        if (manager.layoutKeys[i] == tag)
            return manager.layouts[i];
    }
    return VK_NULL_HANDLE;
}