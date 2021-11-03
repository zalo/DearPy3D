#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "mvTypes.h"

struct mvAssetManager;
struct mvPointLight;

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
    mvSceneBuffer         sceneBuffer;
    //VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet*      descriptorSets;
};

mvScene mvCreateScene(mvAssetManager& am, mvSceneData sceneData);
void    mvUpdateSceneDescriptors(mvAssetManager& am, mvScene& scene, mvPointLight& light);
void    mvBindScene(mvAssetManager& am, mvAssetID scene, mvAssetID pipelineLayout);