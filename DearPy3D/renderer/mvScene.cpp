#include "mvScene.h"
#include "mvAssetManager.h"
#include "mvLights.h"

mvScene 
mvCreateScene(mvAssetManager& am, mvSceneData sceneData)
{
    mv_local_persist i32 id = 0u;
    id++;
    std::string hash = "scene_" + std::string(sceneData.doLighting ? "T" : "F") + std::to_string(id);

    mv_local_persist mvPointLightInfo pointLightInfo{};
    mv_local_persist mvDirectionLightInfo dpointLightInfo{};

    mvScene scene{};
    scene.descriptorSet = mvCreateDescriptorSet(am, mvGetRawDescriptorSetLayoutAsset(&am, "scene"), mvGetPipelineLayoutAssetID(&am, "main_pass"));
    scene.descriptorSet.descriptors.push_back(mvCreateDynamicUniformBufferDescriptor(am, mvCreateDynamicUniformBufferDescriptorSpec(0u), hash, 3, sizeof(mvSceneData), &sceneData));
    scene.descriptorSet.descriptors.push_back(mvCreateDynamicUniformBufferDescriptor(am, mvCreateDynamicUniformBufferDescriptorSpec(1u), hash + "light", 3, sizeof(mvPointLightInfo), &pointLightInfo));
    scene.descriptorSet.descriptors.push_back(mvCreateDynamicUniformBufferDescriptor(am, mvCreateDynamicUniformBufferDescriptorSpec(2u), hash + "dlight", 3, sizeof(mvDirectionLightInfo), &dpointLightInfo));
    scene.descriptorSet.descriptors.push_back(mvCreateTextureDescriptor(am, mvCreateTextureDescriptorSpec(3u)));
    mvRegisterAsset(&am, hash, scene.descriptorSet);

    return scene;
}

void
mvUpdateSceneDescriptors(mvAssetManager& am, mvScene& scene, mvAssetID shadowMap)
{
    VkWriteDescriptorSet descriptorWrites[4];

    scene.descriptorSet.descriptors[0].write.pBufferInfo = &am.buffers[scene.descriptorSet.descriptors[0].bufferID[GContext->graphics.currentImageIndex]].asset.bufferInfo;
    scene.descriptorSet.descriptors[1].write.pBufferInfo = &am.buffers[scene.descriptorSet.descriptors[1].bufferID[GContext->graphics.currentImageIndex]].asset.bufferInfo;
    scene.descriptorSet.descriptors[2].write.pBufferInfo = &am.buffers[scene.descriptorSet.descriptors[2].bufferID[GContext->graphics.currentImageIndex]].asset.bufferInfo;
    scene.descriptorSet.descriptors[3].write.pImageInfo = &am.textures[shadowMap].asset.imageInfo;

    for (u32 i = 0; i < scene.descriptorSet.descriptors.size(); i++)
    {
        scene.descriptorSet.descriptors[i].write.dstSet = scene.descriptorSet.descriptorSets[GContext->graphics.currentImageIndex];
        descriptorWrites[i] = scene.descriptorSet.descriptors[i].write;
    }

    vkUpdateDescriptorSets(mvGetLogicalDevice(), 4, descriptorWrites, 0, nullptr);
}

void
mvBindScene(mvAssetManager& am, mvAssetID sceneId, mvSceneData data, u32 index)
{
    mvScene& scene = am.scenes[sceneId].asset;
    mvPartialUpdateBuffer(am.buffers[scene.descriptorSet.descriptors[0].bufferID[GContext->graphics.currentImageIndex]].asset, &data, index);

    u32 uniformOffsets[3];
    uniformOffsets[0] = index * mvGetRequiredUniformBufferSize(sizeof(mvSceneData));
    uniformOffsets[1] = index * mvGetRequiredUniformBufferSize(sizeof(mvPointLightInfo));
    uniformOffsets[2] = index * mvGetRequiredUniformBufferSize(sizeof(mvDirectionLightInfo));
    mvBindDescriptorSet(am, scene.descriptorSet, 0, 3, uniformOffsets);
}