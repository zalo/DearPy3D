#include "mvMaterials.h"
#include <stdexcept>
#include "mvContext.h"
#include "mvTextures.h"
#include "mvAssetManager.h"

mvMaterial 
mvCreateMaterial(mvAssetManager& am, mvMaterialData materialData, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, const char* vertexShader, const char* pixelShader)
{
    mv_local_persist int temp = 0;
    temp++;
    std::string hash = std::string(vertexShader) + std::string(pixelShader) + std::to_string(temp);

    mvMaterial material{};
    material.descriptorSets = new VkDescriptorSet[GContext->graphics.swapChainImages.size()];
    material.texture = mvGetTextureAsset(&am, "../../Resources/brickwall.jpg");
    material.sampler = mvGetSamplerAsset(&am);

    for (size_t i = 0; i < GContext->graphics.swapChainImages.size(); i++)
        material.materialBuffer.buffers.push_back(mvGetDynamicBufferAsset(&am,
            &materialData, 
            1, 
            sizeof(mvMaterialData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, hash));

    //-----------------------------------------------------------------------------
    // create descriptor set layout
    //-----------------------------------------------------------------------------
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.resize(2);

    bindings[0].binding = 0u;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[0].pImmutableSamplers = nullptr;

    bindings[1].binding = 1u;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[1].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(mvGetLogicalDevice(), &layoutInfo, nullptr, &material.descriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout!");

    //-----------------------------------------------------------------------------
    // allocate descriptor sets
    //-----------------------------------------------------------------------------
    std::vector<VkDescriptorSetLayout> layouts(GContext->graphics.swapChainImages.size(), material.descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = GContext->graphics.descriptorPool;
    allocInfo.descriptorSetCount = GContext->graphics.swapChainImages.size();
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(mvGetLogicalDevice(), &allocInfo, material.descriptorSets) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate descriptor sets!");

    //-----------------------------------------------------------------------------
    // update descriptor sets
    //-----------------------------------------------------------------------------

    for (int i = 0; i < GContext->graphics.swapChainImages.size(); i++)
    {
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        descriptorWrites.resize(2);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = am.textures[material.texture].texture.textureImageView;
        imageInfo.sampler = am.samplers[material.sampler].sampler.textureSampler;

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = material.descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &imageInfo;

        VkDescriptorBufferInfo materialInfo;
        materialInfo.buffer = am.dynBuffers[material.materialBuffer.buffers[i]].buffer.buffer;
        materialInfo.offset = 0;
        materialInfo.range = mvGetRequiredUniformBufferSize(sizeof(mvMaterialData));

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = material.descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &materialInfo;

        vkUpdateDescriptorSets(mvGetLogicalDevice(),
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);

    }

    GContext->graphics.deletionQueue2.pushDeletor([=]() {
          vkDestroyDescriptorSetLayout(mvGetLogicalDevice(), material.descriptorSetLayout, nullptr);
        });

    descriptorSetLayouts.push_back(material.descriptorSetLayout);
    material.pipeline = mvGetPipelineAsset(&am, descriptorSetLayouts, vertexShader, pixelShader);
    return material;
}
