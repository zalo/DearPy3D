#pragma once

#include "mvDearPy3D.h"
#include "mvPipeline.h"
#include "mvTextures.h"
#include "mvBuffer.h"
#include "sMath.h"
#include "mvDescriptors.h"

namespace DearPy3D
{
    mvMaterial        create_material            (mvGraphics& graphics, mvRendererContext& rctx, mvMaterialData materialData, const char* vertexShader, const char* pixelShader);
    void              update_material_descriptors(mvRendererContext& rctx, mvMaterial& material, mvAssetID colorTexture, mvAssetID normalTexture, mvAssetID specularTexture);
    mvMaterialManager create_material_manager();
    void              cleanup_material_manager   (mvGraphics& graphics, mvMaterialManager& manager);
    mvAssetID         register_material          (mvMaterialManager& manager, const std::string& tag, mvMaterial material);
}

struct mvMaterialData
{
    sVec4 materialColor;
    //-------------------------- ( 16 bytes )
};

struct mvMaterial
{
    u32                   offsetIndex;
    mvDescriptorSet       descriptorSet;
    std::string           vertexShader;
    std::string           pixelShader;
};

struct mvMaterialManager
{
    std::string* materialKeys;
    u32          maxMaterialCount;
    u32          materialCount;
    mvMaterial*  materials;
};