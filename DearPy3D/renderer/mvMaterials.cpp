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
    material.texture = mvGetTextureAssetID(&am, "../../Resources/brickwall.jpg");
    material.vertexShader = vertexShader;
    material.pixelShader = pixelShader;

    std::string hash =
        std::string(materialData.useTextureMap ? "T" : "F")
        + std::string(materialData.useNormalMap ? "T" : "F")
        + std::string(materialData.useSpecularMap ? "T" : "F")
        + std::string(materialData.useGlossAlpha ? "T" : "F")
        + std::string(materialData.hasAlpha ? "T" : "F")
        + std::string(materialData.doLighting ? "T" : "F");

    // temporary until we have a "bindless" solution
    // to descriptor set updates
    mv_local_persist int i = 0;
    //i++;
    hash.append(std::to_string(i));

    for (size_t i = 0; i < GContext->graphics.swapChainImages.size(); i++)
    {

        material.materialBuffer.buffers.push_back(mvRegisterAsset(&am, hash, 
            mvCreateDynamicBuffer(&materialData, 1, sizeof(mvMaterialData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)));
    }

    material.descriptorSets = mvCreateDescriptorSet(am, mvGetRawDescriptorSetLayoutAsset(&am, "phong"), mvGetPipelineLayoutAssetID(&am, "main_pass"));
    return material;
}

void
mvUpdateMaterialDescriptors(mvAssetManager& am, mvMaterial& material, mvAssetID colorTexture, mvAssetID normalTexture, mvAssetID specularTexture)
{
    
    std::vector<VkWriteDescriptorSet> descriptorWrites;
    descriptorWrites.resize(4);

    // temporary hack, all descriptors need to be updated
    if (colorTexture == MV_INVALID_ASSET_ID)    colorTexture = material.texture;
    if (normalTexture == MV_INVALID_ASSET_ID)   normalTexture = material.texture;
    if (specularTexture == MV_INVALID_ASSET_ID) specularTexture = material.texture;

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = material.descriptorSets.descriptorSets[GContext->graphics.currentImageIndex];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pImageInfo = &am.textures[colorTexture].asset.imageInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = material.descriptorSets.descriptorSets[GContext->graphics.currentImageIndex];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &am.textures[normalTexture].asset.imageInfo;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = material.descriptorSets.descriptorSets[GContext->graphics.currentImageIndex];
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pImageInfo = &am.textures[specularTexture].asset.imageInfo;

    VkDescriptorBufferInfo materialInfo;
    materialInfo.buffer = am.buffers[material.materialBuffer.buffers[GContext->graphics.currentImageIndex]].asset.buffer;
    materialInfo.offset = 0;
    materialInfo.range = mvGetRequiredUniformBufferSize(sizeof(mvMaterialData));

    descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[3].dstSet = material.descriptorSets.descriptorSets[GContext->graphics.currentImageIndex];
    descriptorWrites[3].dstBinding = 3;
    descriptorWrites[3].dstArrayElement = 0;
    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[3].descriptorCount = 1;
    descriptorWrites[3].pBufferInfo = &materialInfo;

    vkUpdateDescriptorSets(mvGetLogicalDevice(), 4, descriptorWrites.data(), 0, nullptr);
}
