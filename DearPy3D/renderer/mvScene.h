#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "mvTypes.h"

struct mvAssetManager;

struct mvSceneData
{
    b32 doLighting = true;
    char _pad1[12];
    //-------------------------- ( 16 bytes )

    // 
    //-------------------------- ( 1 * 16 = 16 bytes )
};

struct mvSceneBuffer
{
    std::vector<mvAssetID> buffers;
};

struct mvScene
{
    u32                   offsetIndex = 0u;
    mvSceneBuffer         sceneBuffer;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet*      descriptorSets;
};

mvScene mvCreateScene(mvAssetManager& am, mvSceneData sceneData, b32 doLighting);