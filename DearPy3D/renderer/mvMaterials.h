#pragma once

#include <memory>
#include "mvPipeline.h"
#include "mvDeletionQueue.h"
#include "mvTextures.h"
#include "mvSampler.h"
#include "mvBuffer.h"
#include "mvMath.h"
#include "mvObjLoader.h"

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

struct mvMaterialBuffer
{
    std::vector<mvAssetID> buffers;
};

struct mvMaterial
{
    mvPipeline            pipeline;
    mvTexture             texture;
    mvSampler             sampler;
    u32                   offsetIndex = 0u;
    mvMaterialBuffer      materialBuffer;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet*      descriptorSets;
};

mvMaterial mvCreateMaterial   (mvAssetManager& am, mvMaterialData materialData, const char* vertexShader, const char* pixelShader);
void       mvCleanupMaterial  (mvAssetManager& am, mvMaterial& material);
