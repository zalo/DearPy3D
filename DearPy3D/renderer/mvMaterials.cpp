#include "mvMaterials.h"
#include <stdexcept>
#include "mvContext.h"
#include "mvTextures.h"
#include "mvAssetManager.h"

mvMaterial 
mvCreateMaterial(mvAssetManager& am, mvMaterialData materialData, const char* vertexShader, const char* pixelShader)
{
    mv_local_persist int temp = 0;
    temp++;
    std::string hash = std::string(vertexShader) + std::string(pixelShader) + std::to_string(temp);

    mvMaterial material{};
    material.descriptorSets = new VkDescriptorSet[GContext->graphics.swapChainImages.size()];
    material.texture = mvGetTextureAsset(&am, "../../Resources/brickwall.jpg");
    material.sampler = mvGetSamplerAsset(&am);
    material.vertexShader = vertexShader;
    material.pixelShader = pixelShader;

    for (size_t i = 0; i < GContext->graphics.swapChainImages.size(); i++)
        material.materialBuffer.buffers.push_back(mvGetDynamicBufferAsset(&am,
            &materialData, 
            1, 
            sizeof(mvMaterialData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, hash));

    //-----------------------------------------------------------------------------
    // allocate descriptor sets
    //-----------------------------------------------------------------------------
    std::vector<VkDescriptorSetLayout> layouts(GContext->graphics.swapChainImages.size(), am.descriptorSetLayouts[mvGetDescriptorSetLayoutAsset(&am, "phong")].layout);
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
    return material;
}
