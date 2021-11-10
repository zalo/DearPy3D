#include "mvMaterials.h"
#include <stdexcept>
#include "mvContext.h"
#include "mvTextures.h"
#include "mvAssetManager.h"

mvMaterial 
mvCreateMaterial(mvAssetManager& am, mvMaterialData materialData, const char* vertexShader, const char* pixelShader)
{
    mvMaterial material{};
    material.data = materialData;
    material.descriptorSets = new VkDescriptorSet[GContext->graphics.swapChainImages.size()];
    material.texture = mvGetTextureAsset(&am, "../../Resources/brickwall.jpg");
    material.vertexShader = vertexShader;
    material.pixelShader = pixelShader;

    std::string hash = mvCreateHash(materialData);

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
        descriptorWrites.resize(4);

        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = am.textures[material.texture].texture.imageInfo.imageView;
            imageInfo.sampler = am.textures[material.texture].texture.imageInfo.sampler;

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = material.descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pImageInfo = &imageInfo;
        }

        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = am.textures[material.texture].texture.imageInfo.imageView;
            imageInfo.sampler = am.textures[material.texture].texture.imageInfo.sampler;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = material.descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;
        }

        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = am.textures[material.texture].texture.imageInfo.imageView;
            imageInfo.sampler = am.textures[material.texture].texture.imageInfo.sampler;

            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[2].dstSet = material.descriptorSets[i];
            descriptorWrites[2].dstBinding = 2;
            descriptorWrites[2].dstArrayElement = 0;
            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[2].descriptorCount = 1;
            descriptorWrites[2].pImageInfo = &imageInfo;
        }

        VkDescriptorBufferInfo materialInfo;
        materialInfo.buffer = am.dynBuffers[material.materialBuffer.buffers[i]].buffer.buffer;
        materialInfo.offset = 0;
        materialInfo.range = mvGetRequiredUniformBufferSize(sizeof(mvMaterialData));

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = material.descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pBufferInfo = &materialInfo;

        vkUpdateDescriptorSets(mvGetLogicalDevice(),
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);

    }
    return material;
}

void
mvUpdateMaterialDescriptors(mvAssetManager& am, mvMaterial& material, mvAssetID colorTexture, mvAssetID normalTexture, mvAssetID specularTexture)
{
    
    std::vector<VkWriteDescriptorSet> descriptorWrites;
    descriptorWrites.resize(4);

    s32 currentIndex = 0;

    if (colorTexture > -1)
    {
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = material.descriptorSets[GContext->graphics.currentImageIndex];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &am.textures[colorTexture].texture.imageInfo;
        currentIndex++;
    }

    if (normalTexture > -1)
    {
        descriptorWrites[currentIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[currentIndex].dstSet = material.descriptorSets[GContext->graphics.currentImageIndex];
        descriptorWrites[currentIndex].dstBinding = 1;
        descriptorWrites[currentIndex].dstArrayElement = 0;
        descriptorWrites[currentIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[currentIndex].descriptorCount = 1;
        descriptorWrites[currentIndex].pImageInfo = &am.textures[normalTexture].texture.imageInfo;
        currentIndex++;
    }

    if (specularTexture > -1)
    {
        descriptorWrites[currentIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[currentIndex].dstSet = material.descriptorSets[GContext->graphics.currentImageIndex];
        descriptorWrites[currentIndex].dstBinding = 2;
        descriptorWrites[currentIndex].dstArrayElement = 0;
        descriptorWrites[currentIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[currentIndex].descriptorCount = 1;
        descriptorWrites[currentIndex].pImageInfo = &am.textures[specularTexture].texture.imageInfo;
        currentIndex++;
    }

    VkDescriptorBufferInfo materialInfo;
    materialInfo.buffer = am.dynBuffers[material.materialBuffer.buffers[GContext->graphics.currentImageIndex]].buffer.buffer;
    materialInfo.offset = 0;
    materialInfo.range = mvGetRequiredUniformBufferSize(sizeof(mvMaterialData));

    descriptorWrites[currentIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[currentIndex].dstSet = material.descriptorSets[GContext->graphics.currentImageIndex];
    descriptorWrites[currentIndex].dstBinding = 3;
    descriptorWrites[currentIndex].dstArrayElement = 0;
    descriptorWrites[currentIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[currentIndex].descriptorCount = 1;
    descriptorWrites[currentIndex].pBufferInfo = &materialInfo;
    currentIndex++;

    vkUpdateDescriptorSets(mvGetLogicalDevice(), static_cast<uint32_t>(currentIndex), descriptorWrites.data(), 0, nullptr);
}
