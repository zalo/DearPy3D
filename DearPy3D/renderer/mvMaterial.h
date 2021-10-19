#pragma once

#include <memory>
#include "mvPipeline.h"
#include "mvDeletionQueue.h"
#include "mvTexture.h"
#include "mvSampler.h"
#include "mvBuffer.h"
#include "mvMath.h"
#include "mvObjLoader.h"

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
    std::vector<mvBuffer> buffers;
};

struct mvMaterial
{
    mvPipeline            pipeline;
    mvTexture             texture;
    mvSampler             sampler;
    uint32_t              offsetIndex = 0u;
    mvMaterialBuffer      materialBuffer;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet*      descriptorSets;
};

mvMaterial mvCreateMaterial   (mvMaterialData materialData, const char* vertexShader, const char* pixelShader);
mvMaterial mvCreateObjMaterial(mvMaterialData materialData, mvObjMaterial objMaterial, const char* vertexShader, const char* pixelShader, const std::string& path);
void       mvCleanupMaterial  (mvMaterial& material);
