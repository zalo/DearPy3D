#include "mvMaterials.h"
#include <stdexcept>
#include "mvGraphics.h"
#include "mvTextures.h"
#include "mvAssetManager.h"

mvMaterial 
create_material(mvGraphics& graphics, mvDescriptorManager& dsManager, mvPipelineManager& pmManager, mvMaterialData materialData, const char* vertexShader, const char* pixelShader)
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

    material.descriptorSet = create_descriptor_set(graphics, get_descriptor_set_layout(dsManager, "phong"), get_pipeline_layout(pmManager, "primary_pass"));
    material.descriptorSet.descriptors.push_back(create_texture_descriptor(create_texture_descriptor_spec(0u)));
    material.descriptorSet.descriptors.push_back(create_texture_descriptor(create_texture_descriptor_spec(1u)));
    material.descriptorSet.descriptors.push_back(create_texture_descriptor(create_texture_descriptor_spec(2u)));
    material.descriptorSet.descriptors.push_back(create_uniform_descriptor(graphics, create_uniform_descriptor_spec(3u), hash, sizeof(mvMaterialData), &materialData));
    register_descriptor_set(dsManager, hash, material.descriptorSet);
    return material;
}

void
update_material_descriptors(mvGraphics& graphics, mvAssetManager& am, mvMaterial& material, mvAssetID colorTexture, mvAssetID normalTexture, mvAssetID specularTexture)
{
    
    VkWriteDescriptorSet descriptorWrites[4];

    // temporary hack, all descriptors need to be updated
    if (colorTexture    == MV_INVALID_ASSET_ID) colorTexture = 0;
    if (normalTexture   == MV_INVALID_ASSET_ID) normalTexture = 0;
    if (specularTexture == MV_INVALID_ASSET_ID) specularTexture = 0;

    material.descriptorSet.descriptors[0].write.pImageInfo = &am.textures[colorTexture].asset.imageInfo;
    material.descriptorSet.descriptors[1].write.pImageInfo = &am.textures[normalTexture].asset.imageInfo;
    material.descriptorSet.descriptors[2].write.pImageInfo = &am.textures[specularTexture].asset.imageInfo;
    material.descriptorSet.descriptors[3].write.pBufferInfo = &material.descriptorSet.descriptors[3].buffers[graphics.currentImageIndex].bufferInfo;
    
    // set descriptor sets
    for (u32 i = 0; i < material.descriptorSet.descriptors.size(); i++)
    {
        material.descriptorSet.descriptors[i].write.dstSet = material.descriptorSet.descriptorSets[graphics.currentImageIndex];
        descriptorWrites[i] = material.descriptorSet.descriptors[i].write;
    }

    vkUpdateDescriptorSets(graphics.logicalDevice, 4, descriptorWrites, 0, nullptr);
}

mvMaterialManager
create_material_manager()
{
    mvMaterialManager manager{};

    manager.materials = new mvMaterial[manager.maxMaterialCount];
    manager.materialKeys = new std::string[manager.maxMaterialCount];

    for (u32 i = 0; i < manager.maxMaterialCount; i++)
    {
        manager.materialKeys[i] = "";
    }

    return manager;
}

void
cleanup_material_manager(mvGraphics& graphics, mvMaterialManager& manager)
{
    manager.maxMaterialCount = 0u;
    delete[] manager.materials;
    delete[] manager.materialKeys;
}

mvAssetID
register_material(mvMaterialManager& manager, const std::string& tag, mvMaterial material)
{
    assert(manager.materialCount <= manager.maxMaterialCount);
    manager.materials[manager.materialCount] = material;
    manager.materialKeys[manager.materialCount] = tag;
    manager.materialCount++;
    return manager.materialCount - 1;
}