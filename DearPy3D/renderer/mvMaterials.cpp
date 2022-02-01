#include "mvMaterials.h"
#include <stdexcept>
#include "mvGraphics.h"
#include "mvTextures.h"
#include "mvRenderer.h"

mvMaterial 
DearPy3D::create_material(mvGraphics& graphics, mvRendererContext& rctx, mvMaterialData materialData, const char* vertexShader, const char* pixelShader)
{
    std::string hash = "solid_color";
    mvMaterial material{};
    material.offsetIndex = 0u;
    material.vertexShader = vertexShader;
    material.pixelShader = pixelShader;
    material.descriptorSet = create_descriptor_set(graphics, get_descriptor_set_layout(rctx.descriptorManager, "phong"), get_pipeline_layout(rctx.pipelineManager, "primary_pass"));
    material.descriptorSet.descriptors.push_back(create_texture_descriptor(create_texture_descriptor_spec(0u)));
    material.descriptorSet.descriptors.push_back(create_texture_descriptor(create_texture_descriptor_spec(1u)));
    material.descriptorSet.descriptors.push_back(create_texture_descriptor(create_texture_descriptor_spec(2u)));
    material.descriptorSet.descriptors.push_back(create_uniform_descriptor(graphics, create_uniform_descriptor_spec(3u), hash, sizeof(mvMaterialData), &materialData));
    register_descriptor_set(rctx.descriptorManager, hash, material.descriptorSet);
    return material;
}

void
DearPy3D::update_material_descriptors(mvRendererContext& rctx, mvMaterial& material, mvAssetID colorTexture, mvAssetID normalTexture, mvAssetID specularTexture)
{
    
    VkWriteDescriptorSet descriptorWrites[4];

    // temporary hack, all descriptors need to be updated
    if (colorTexture    == MV_INVALID_ASSET_ID) colorTexture = 0;
    if (normalTexture   == MV_INVALID_ASSET_ID) normalTexture = 0;
    if (specularTexture == MV_INVALID_ASSET_ID) specularTexture = 0;

    material.descriptorSet.descriptors[0].write.pImageInfo = &rctx.textureManager.textures[colorTexture].imageInfo;
    material.descriptorSet.descriptors[1].write.pImageInfo = &rctx.textureManager.textures[normalTexture].imageInfo;
    material.descriptorSet.descriptors[2].write.pImageInfo = &rctx.textureManager.textures[specularTexture].imageInfo;
    material.descriptorSet.descriptors[3].write.pBufferInfo = &material.descriptorSet.descriptors[3].buffers[rctx.graphics->currentImageIndex].bufferInfo;
    
    // set descriptor sets
    for (u32 i = 0; i < material.descriptorSet.descriptors.size(); i++)
    {
        material.descriptorSet.descriptors[i].write.dstSet = material.descriptorSet.descriptorSets[rctx.graphics->currentImageIndex];
        descriptorWrites[i] = material.descriptorSet.descriptors[i].write;
    }

    vkUpdateDescriptorSets(rctx.graphics->logicalDevice, 4, descriptorWrites, 0, nullptr);
}

mvMaterialManager
DearPy3D::create_material_manager()
{
    mvMaterialManager manager{};
    manager.materialKeys = nullptr;
    manager.maxMaterialCount = 500u;
    manager.materialCount = 0u;
    manager.materials = nullptr;
    manager.materials = new mvMaterial[manager.maxMaterialCount];
    manager.materialKeys = new std::string[manager.maxMaterialCount];

    for (u32 i = 0; i < manager.maxMaterialCount; i++)
    {
        manager.materialKeys[i] = "";
    }

    return manager;
}

void
DearPy3D::cleanup_material_manager(mvGraphics& graphics, mvMaterialManager& manager)
{
    manager.materialCount = 0u;
    delete[] manager.materials;
    delete[] manager.materialKeys;
}

mvAssetID
DearPy3D::register_material(mvMaterialManager& manager, const std::string& tag, mvMaterial material)
{
    assert(manager.materialCount <= manager.maxMaterialCount);
    manager.materials[manager.materialCount] = material;
    manager.materialKeys[manager.materialCount] = tag;
    manager.materialCount++;
    return manager.materialCount - 1;
}