#include "mvScene.h"
#include "mvAssetManager.h"

mvScene 
mvCreateScene(mvAssetManager& am, mvSceneData sceneData, b32 doLighting)
{
    std::string hash = "scene_" + std::string(doLighting ? "T" : "F");

    mvScene scene{};
    scene.descriptorSets = new VkDescriptorSet[GContext->graphics.swapChainImages.size()];

    for (size_t i = 0; i < GContext->graphics.swapChainImages.size(); i++)
        scene.sceneBuffer.buffers.push_back(mvGetDynamicBufferAsset(&am,
            &sceneData,
            1,
            sizeof(mvMaterialData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, hash));

    //-----------------------------------------------------------------------------
    // create descriptor set layout
    //-----------------------------------------------------------------------------
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.resize(1);

    bindings[0].binding = 0u;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[0].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(mvGetLogicalDevice(), &layoutInfo, nullptr, &scene.descriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout!");

    //-----------------------------------------------------------------------------
    // allocate descriptor sets
    //-----------------------------------------------------------------------------
    std::vector<VkDescriptorSetLayout> layouts(GContext->graphics.swapChainImages.size(), scene.descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = GContext->graphics.descriptorPool;
    allocInfo.descriptorSetCount = GContext->graphics.swapChainImages.size();
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(mvGetLogicalDevice(), &allocInfo, scene.descriptorSets) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate descriptor sets!");

    //-----------------------------------------------------------------------------
    // update descriptor sets
    //-----------------------------------------------------------------------------

    for (int i = 0; i < GContext->graphics.swapChainImages.size(); i++)
    {
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        descriptorWrites.resize(1);

        VkDescriptorBufferInfo materialInfo;
        materialInfo.buffer = am.dynBuffers[scene.sceneBuffer.buffers[i]].buffer.buffer;
        materialInfo.offset = 0;
        materialInfo.range = mvGetRequiredUniformBufferSize(sizeof(mvSceneData));

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = scene.descriptorSets[i];
        descriptorWrites[1].dstBinding = 0;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &materialInfo;

        vkUpdateDescriptorSets(mvGetLogicalDevice(),
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);

    }

    GContext->graphics.deletionQueue2.pushDeletor([=]() {
        vkDestroyDescriptorSetLayout(mvGetLogicalDevice(), scene.descriptorSetLayout, nullptr);
        });

    return scene;
}