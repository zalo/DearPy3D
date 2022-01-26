#include "mvScene.h"
#include "mvLights.h"
#include "mvGraphics.h"
#include "mvRenderer.h"

mvScene 
create_scene(mvRendererContext& rctx, mvSceneData sceneData)
{
    mv_local_persist i32 id = 0u;
    id++;
    std::string hash = "scene_" + std::string(sceneData.doLighting ? "T" : "F") + std::to_string(id);

    mv_local_persist mvPointLightInfo pointLightInfo{};
    mv_local_persist mvDirectionLightInfo dpointLightInfo{};

    VkDescriptorSetLayout dsLayout = get_descriptor_set_layout(rctx.descriptorManager, "scene");
    VkPipelineLayout pLayout = get_pipeline_layout(rctx.pipelineManager, "primary_pass");

    mvScene scene{};
    scene.descriptorSet = create_descriptor_set(*rctx.graphics, dsLayout, pLayout);
    scene.descriptorSet.descriptors.push_back(create_dynamic_uniform_descriptor(*rctx.graphics, create_dynamic_uniform_descriptor_spec(0u), hash, 3, sizeof(mvSceneData), &sceneData));
    scene.descriptorSet.descriptors.push_back(create_dynamic_uniform_descriptor(*rctx.graphics, create_dynamic_uniform_descriptor_spec(1u), hash + "light", 3, sizeof(mvPointLightInfo), &pointLightInfo));
    scene.descriptorSet.descriptors.push_back(create_dynamic_uniform_descriptor(*rctx.graphics, create_dynamic_uniform_descriptor_spec(2u), hash + "dlight", 3, sizeof(mvDirectionLightInfo), &dpointLightInfo));
    scene.descriptorSet.descriptors.push_back(create_texture_descriptor(create_texture_descriptor_spec(3u)));
    scene.descriptorSet.descriptors.push_back(create_texture_descriptor(create_texture_descriptor_spec(4u)));
    register_descriptor_set(rctx.descriptorManager, hash, scene.descriptorSet);

    return scene;
}

void
update_scene_descriptors(mvRendererContext& rctx, mvScene& scene, mvTexture& shadowMap, mvAssetID shadowCubeMap)
{
    VkWriteDescriptorSet descriptorWrites[5];

    scene.descriptorSet.descriptors[0].write.pBufferInfo = &scene.descriptorSet.descriptors[0].buffers[rctx.graphics->currentImageIndex].bufferInfo;
    scene.descriptorSet.descriptors[1].write.pBufferInfo = &scene.descriptorSet.descriptors[1].buffers[rctx.graphics->currentImageIndex].bufferInfo;
    scene.descriptorSet.descriptors[2].write.pBufferInfo = &scene.descriptorSet.descriptors[2].buffers[rctx.graphics->currentImageIndex].bufferInfo;
    scene.descriptorSet.descriptors[3].write.pImageInfo = &shadowMap.imageInfo;
    scene.descriptorSet.descriptors[4].write.pImageInfo = &rctx.textureManager.textures[shadowCubeMap].imageInfo;

    for (u32 i = 0; i < scene.descriptorSet.descriptors.size(); i++)
    {
        scene.descriptorSet.descriptors[i].write.dstSet = scene.descriptorSet.descriptorSets[rctx.graphics->currentImageIndex];
        descriptorWrites[i] = scene.descriptorSet.descriptors[i].write;
    }

    vkUpdateDescriptorSets(rctx.graphics->logicalDevice, 5, descriptorWrites, 0, nullptr);
}

void
bind_scene(mvGraphics& graphics, mvScene& scene, mvSceneData data, u32 index)
{
    partial_buffer_update(graphics, scene.descriptorSet.descriptors[0].buffers[graphics.currentImageIndex], &data, index);

    u32 uniformOffsets[3];
    uniformOffsets[0] = index * get_required_uniform_buffer_size(graphics, sizeof(mvSceneData));
    uniformOffsets[1] = index * get_required_uniform_buffer_size(graphics, sizeof(mvPointLightInfo));
    uniformOffsets[2] = index * get_required_uniform_buffer_size(graphics, sizeof(mvDirectionLightInfo));
    bind_descriptor_set(graphics, scene.descriptorSet, 0, 3, uniformOffsets);
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

void
cleanup_model(mvGraphics& graphics, mvModel& model)
{
    vkDeviceWaitIdle(graphics.logicalDevice);

    model.sceneCount = 0u;

    for (int i = 0; i < model.meshCount; i++)
    {
        vkDestroyBuffer(graphics.logicalDevice, model.meshes[i].indexBuffer.buffer, nullptr);
        vkDestroyBuffer(graphics.logicalDevice, model.meshes[i].vertexBuffer.buffer, nullptr);
        vkFreeMemory(graphics.logicalDevice, model.meshes[i].indexBuffer.deviceMemory, nullptr);
        vkFreeMemory(graphics.logicalDevice, model.meshes[i].vertexBuffer.deviceMemory, nullptr);

        model.meshes[i].indexBuffer.buffer = VK_NULL_HANDLE;
        model.meshes[i].indexBuffer.deviceMemory = VK_NULL_HANDLE;
        model.meshes[i].indexBuffer.specification.count = 0u;
        model.meshes[i].indexBuffer.specification.aligned = false;
        model.meshes[i].vertexBuffer.buffer = VK_NULL_HANDLE;
        model.meshes[i].vertexBuffer.deviceMemory = VK_NULL_HANDLE;
        model.meshes[i].vertexBuffer.specification.count = 0u;
        model.meshes[i].vertexBuffer.specification.aligned = false;
    }

    delete[] model.meshes;
    delete[] model.scenes;
    delete[] model.nodes;

    model.meshes = nullptr;
    model.scenes = nullptr;
    model.nodes = nullptr;

    model.meshCount = 0u;
    model.sceneCount = 0u;
    model.nodeCount = 0u;
}