#include "mvMaterials.h"
#include <stdexcept>
#include "mvContext.h"
#include "mvTextures.h"
#include "mvAssetManager.h"

mvMaterial 
mvCreateMaterial(mvAssetManager& am, mvMaterialData materialData, const char* vertexShader, const char* pixelShader)
{
    mvMaterial material{};
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

    material.descriptorSet = mvCreateDescriptorSet(am, mvGetRawDescriptorSetLayoutAsset(&am, "phong"), mvGetPipelineLayoutAssetID(&am, "primary_pass"));
    material.descriptorSet.descriptors.push_back(mvCreateTextureDescriptor(am, mvCreateTextureDescriptorSpec(0u)));
    material.descriptorSet.descriptors.push_back(mvCreateTextureDescriptor(am, mvCreateTextureDescriptorSpec(1u)));
    material.descriptorSet.descriptors.push_back(mvCreateTextureDescriptor(am, mvCreateTextureDescriptorSpec(2u)));
    material.descriptorSet.descriptors.push_back(mvCreateUniformBufferDescriptor(am, mvCreateUniformBufferDescriptorSpec(3u), hash, sizeof(mvMaterialData), &materialData));
    mvRegisterAsset(&am, hash, material.descriptorSet);
    return material;
}

void
mvUpdateMaterialDescriptors(mvAssetManager& am, mvMaterial& material, mvAssetID colorTexture, mvAssetID normalTexture, mvAssetID specularTexture)
{
    
    VkWriteDescriptorSet descriptorWrites[4];

    // temporary hack, all descriptors need to be updated
    if (colorTexture    == MV_INVALID_ASSET_ID) colorTexture = 0;
    if (normalTexture   == MV_INVALID_ASSET_ID) normalTexture = 0;
    if (specularTexture == MV_INVALID_ASSET_ID) specularTexture = 0;

    material.descriptorSet.descriptors[0].write.pImageInfo = &am.textures[colorTexture].asset.imageInfo;
    material.descriptorSet.descriptors[1].write.pImageInfo = &am.textures[normalTexture].asset.imageInfo;
    material.descriptorSet.descriptors[2].write.pImageInfo = &am.textures[specularTexture].asset.imageInfo;
    material.descriptorSet.descriptors[3].write.pBufferInfo = &am.buffers[material.descriptorSet.descriptors[3].bufferID[GContext->graphics.currentImageIndex]].asset.bufferInfo;
    
    // set descriptor sets
    for (u32 i = 0; i < material.descriptorSet.descriptors.size(); i++)
    {
        material.descriptorSet.descriptors[i].write.dstSet = material.descriptorSet.descriptorSets[GContext->graphics.currentImageIndex];
        descriptorWrites[i] = material.descriptorSet.descriptors[i].write;
    }

    vkUpdateDescriptorSets(mvGetLogicalDevice(), 4, descriptorWrites, 0, nullptr);
}
