#include "mvScene.h"
#include "mvAssetManager.h"
#include "mvLights.h"
#include "mvGraphics.h"

mvScene 
create_scene(mvGraphics& graphics, mvAssetManager& am, mvSceneData sceneData)
{
    mv_local_persist i32 id = 0u;
    id++;
    std::string hash = "scene_" + std::string(sceneData.doLighting ? "T" : "F") + std::to_string(id);

    mv_local_persist mvPointLightInfo pointLightInfo{};
    mv_local_persist mvDirectionLightInfo dpointLightInfo{};

    mvScene scene{};
    scene.descriptorSet = create_descriptor_set(graphics, am, mvGetRawDescriptorSetLayoutAsset(&am, "scene"), mvGetPipelineLayoutAssetID(&am, "primary_pass"));
    scene.descriptorSet.descriptors.push_back(create_dynamic_uniform_descriptor(graphics, am, create_dynamic_uniform_descriptor_spec(0u), hash, 3, sizeof(mvSceneData), &sceneData));
    scene.descriptorSet.descriptors.push_back(create_dynamic_uniform_descriptor(graphics, am, create_dynamic_uniform_descriptor_spec(1u), hash + "light", 3, sizeof(mvPointLightInfo), &pointLightInfo));
    scene.descriptorSet.descriptors.push_back(create_dynamic_uniform_descriptor(graphics, am, create_dynamic_uniform_descriptor_spec(2u), hash + "dlight", 3, sizeof(mvDirectionLightInfo), &dpointLightInfo));
    scene.descriptorSet.descriptors.push_back(create_texture_descriptor(am, create_texture_descriptor_spec(3u)));
    scene.descriptorSet.descriptors.push_back(create_texture_descriptor(am, create_texture_descriptor_spec(4u)));
    mvRegisterAsset(&am, hash, scene.descriptorSet);

    return scene;
}

void
update_scene_descriptors(mvGraphics& graphics, mvAssetManager& am, mvScene& scene, mvAssetID shadowMap, mvAssetID shadowCubeMap)
{
    VkWriteDescriptorSet descriptorWrites[5];

    scene.descriptorSet.descriptors[0].write.pBufferInfo = &am.buffers[scene.descriptorSet.descriptors[0].bufferID[graphics.currentImageIndex]].asset.bufferInfo;
    scene.descriptorSet.descriptors[1].write.pBufferInfo = &am.buffers[scene.descriptorSet.descriptors[1].bufferID[graphics.currentImageIndex]].asset.bufferInfo;
    scene.descriptorSet.descriptors[2].write.pBufferInfo = &am.buffers[scene.descriptorSet.descriptors[2].bufferID[graphics.currentImageIndex]].asset.bufferInfo;
    scene.descriptorSet.descriptors[3].write.pImageInfo = &am.textures[shadowMap].asset.imageInfo;
    scene.descriptorSet.descriptors[4].write.pImageInfo = &am.textures[shadowCubeMap].asset.imageInfo;

    for (u32 i = 0; i < scene.descriptorSet.descriptors.size(); i++)
    {
        scene.descriptorSet.descriptors[i].write.dstSet = scene.descriptorSet.descriptorSets[graphics.currentImageIndex];
        descriptorWrites[i] = scene.descriptorSet.descriptors[i].write;
    }

    vkUpdateDescriptorSets(graphics.logicalDevice, 5, descriptorWrites, 0, nullptr);
}

void
bind_scene(mvGraphics& graphics, mvAssetManager& am, mvAssetID sceneId, mvSceneData data, u32 index)
{
    mvScene& scene = am.scenes[sceneId].asset;
    partial_buffer_update(graphics, am.buffers[scene.descriptorSet.descriptors[0].bufferID[graphics.currentImageIndex]].asset, &data, index);

    u32 uniformOffsets[3];
    uniformOffsets[0] = index * get_required_uniform_buffer_size(graphics, sizeof(mvSceneData));
    uniformOffsets[1] = index * get_required_uniform_buffer_size(graphics, sizeof(mvPointLightInfo));
    uniformOffsets[2] = index * get_required_uniform_buffer_size(graphics, sizeof(mvDirectionLightInfo));
    bind_descriptor_set(graphics, am, scene.descriptorSet, 0, 3, uniformOffsets);
}

void
show_scene_controls(const char* windowName, mvSceneData& sceneData)
{
    ImGui::Begin(windowName);
    ImGui::Checkbox("Diffuse Mapping", (bool*)&sceneData.doDiffuse);
    ImGui::Checkbox("Normal Mapping", (bool*)&sceneData.doNormal);
    ImGui::Checkbox("Specular Mapping", (bool*)&sceneData.doSpecular);
    ImGui::Checkbox("Omni Shadows", (bool*)&sceneData.doOmniShadows);
    ImGui::Checkbox("Direct Shadows", (bool*)&sceneData.doDirectionalShadows);
    ImGui::Checkbox("Skybox", (bool*)&sceneData.doSkybox);
    ImGui::Checkbox("PCF", (bool*)&sceneData.doPCF);
    ImGui::SliderInt("pcfRange", &sceneData.pcfRange, 1, 5);
    ImGui::End();
}