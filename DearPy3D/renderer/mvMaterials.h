#pragma once

#include "mvPipeline.h"
#include "mvTextures.h"
#include "mvBuffer.h"
#include "mvMath.h"
#include "mvObjLoader.h"
#include "mvDescriptors.h"

struct mvAssetManager;

struct mvMaterialData
{
    mvVec4 materialColor = { 0.45f, 0.45f, 0.85f, 1.0f };
    //-------------------------- ( 16 bytes )

    mvVec3 specularColor = { 0.18f, 0.18f, 0.18f };
    float specularGloss = 8.0f;
    //-------------------------- ( 16 bytes )

    float normalMapWeight = 1.0f;
    int useTextureMap = false;
    int useNormalMap = false;
    int useSpecularMap = false;
    //-------------------------- ( 16 bytes )

    int useGlossAlpha = false;
    int hasAlpha = false;
    int doLighting = true;
    char _pad1[4];
    //-------------------------- ( 16 bytes )
        
    // 
    //-------------------------- ( 4 * 16 = 64 bytes )
};

struct mvMaterial
{
    mvAssetID             pipeline;
    u32                   offsetIndex = 0u;
    mvDescriptorSet       descriptorSet;
    std::string           vertexShader;
    std::string           pixelShader;
};

mvMaterial mvCreateMaterial           (mvAssetManager& am, mvMaterialData materialData, const char* vertexShader, const char* pixelShader);
void       mvUpdateMaterialDescriptors(mvAssetManager& am, mvMaterial& material, mvAssetID colorTexture, mvAssetID normalTexture, mvAssetID specularTexture);
