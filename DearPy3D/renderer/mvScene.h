#pragma once

#include <vector>
#include <string>
#include "mvMath.h"
#include <vulkan/vulkan.h>
#include "mvTypes.h"
#include "mvDescriptors.h"

struct mvAssetManager;
struct mvPointLight;
struct mvDirectionLight;

struct mvNode
{
    std::string name;
    mvAssetID   mesh = -1;
    mvAssetID   children[256];
    u32         childCount = 0u;
    mvMat4      matrix = mvIdentityMat4();
    mvVec3      translation = { 0.0f, 0.0f, 0.0f };
    mvVec4      rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
    mvVec3      scale = { 1.0f, 1.0f, 1.0f };
};

struct mvSceneData
{
    b32 doLighting = true;
    b32 doNormal = true;
    b32 doSpecular = true;
    b32 doDiffuse = true;
    //-------------------------- ( 16 bytes )

    // 
    //-------------------------- ( 1 * 16 = 16 bytes )
};

struct mvScene
{
    std::vector<mvAssetID> buffers;
    mvDescriptorSet        descriptorSets;
    mvAssetID              nodes[256];
    u32                    nodeCount = 0u;
    u32                    meshOffset = 0u;
    mvSceneData            data;
};

mvScene mvCreateScene(mvAssetManager& am, mvSceneData sceneData);
void    mvUpdateSceneDescriptors(mvAssetManager& am, mvScene& scene, mvPointLight& light, mvDirectionLight& dlight);
void    mvBindScene(mvAssetManager& am, mvAssetID scene);