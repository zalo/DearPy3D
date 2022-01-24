#pragma once

#include <vector>
#include <string>
#include "mvMath.h"
#include <vulkan/vulkan.h>
#include "mvTypes.h"
#include "mvDescriptors.h"

// forward declarations
struct mvGraphics;
struct mvAssetManager;
struct mvPointLight;
struct mvDirectionLight;
struct mvNode;
struct mvSceneData;
struct mvScene;

mvScene create_scene(mvGraphics& graphics, mvAssetManager& am, mvSceneData sceneData);
void    update_scene_descriptors(mvGraphics& graphics, mvAssetManager& am, mvScene& scene, mvAssetID shadowMap, mvAssetID shadowCubeMap);
void    bind_scene(mvGraphics& graphics, mvAssetManager& am, mvAssetID scene, mvSceneData data, u32 index);
void    show_scene_controls(const char* windowName, mvSceneData& sceneData);

struct mvNode
{
    std::string name;
    mvAssetID   mesh = -1;
    mvAssetID   children[256];
    u32         childCount = 0u;
    mvMat4      matrix = mvMat4(1.0f);
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

    b32 doOmniShadows = true;
    b32 doDirectionalShadows = true;
    b32 doSkybox = true;
    b32 doPCF= false;
    //-------------------------- ( 16 bytes )
     
    mvVec3 camPos = {0.0f, 0.0f, 0.0f};
    i32 pcfRange = 1;
    //-------------------------- ( 16 bytes )

    mvMat4 pointShadowView = mvMat4(1.0f);
    //-------------------------- ( 64 bytes )

    mvMat4 directionalShadowView = mvMat4(1.0f);
    //-------------------------- ( 64 bytes )

    mvMat4 directionalShadowProjection = mvMat4(1.0f);
    //-------------------------- ( 64 bytes )

    //-------------------------- ( 2 * 16 + 3*64= 224 bytes )
};

struct mvScene
{
    mvDescriptorSet descriptorSet;
    mvAssetID       nodes[256];
    u32             nodeCount = 0u;
    u32             meshOffset = 0u;
};