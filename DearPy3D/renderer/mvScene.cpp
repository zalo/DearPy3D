#include "mvScene.h"
#include "mvAssetManager.h"
#include "mvLights.h"

mvScene 
mvCreateScene(mvAssetManager& am, mvSceneData sceneData)
{
    std::string hash = "scene_" + std::string(sceneData.doLighting ? "T" : "F");

    mvScene scene{};
    scene.data = sceneData;
    scene.descriptorSets = new VkDescriptorSet[GContext->graphics.swapChainImages.size()];

    for (size_t i = 0; i < GContext->graphics.swapChainImages.size(); i++)
        scene.sceneBuffer.buffers.push_back(mvGetDynamicBufferAsset(&am,
            &sceneData,
            1,
            sizeof(mvSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, hash));

    //-----------------------------------------------------------------------------
    // create descriptor set layout
    //-----------------------------------------------------------------------------

    if (mvGetPipelineLayoutAsset(&am, "layout_2") == -1)
    {

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        descriptorSetLayouts.resize(2);

        {
            //-----------------------------------------------------------------------------
            // create descriptor set layout
            //-----------------------------------------------------------------------------
            std::vector<VkDescriptorSetLayoutBinding> bindings;
            bindings.resize(3);

            bindings[0].binding = 0u;
            bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindings[0].descriptorCount = 1;
            bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[0].pImmutableSamplers = nullptr;

            bindings[1].binding = 1u;
            bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindings[1].descriptorCount = 1;
            bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[1].pImmutableSamplers = nullptr;

            bindings[2].binding = 2u;
            bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindings[2].descriptorCount = 1;
            bindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[2].pImmutableSamplers = nullptr;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<u32>(bindings.size());
            layoutInfo.pBindings = bindings.data();

            if (vkCreateDescriptorSetLayout(mvGetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayouts[0]) != VK_SUCCESS)
                throw std::runtime_error("failed to create descriptor set layout!");

            mvRegisterDescriptorSetLayoutAsset(&am, descriptorSetLayouts[0], "scene");
        }

        {
            //-----------------------------------------------------------------------------
            // create descriptor set layout
            //-----------------------------------------------------------------------------
            std::vector<VkDescriptorSetLayoutBinding> bindings;
            bindings.resize(4);

            bindings[0].binding = 0u;
            bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[0].descriptorCount = 1;
            bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[0].pImmutableSamplers = nullptr;

            bindings[1].binding = 1u;
            bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[1].descriptorCount = 1;
            bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[1].pImmutableSamplers = nullptr;

            bindings[2].binding = 2u;
            bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[2].descriptorCount = 1;
            bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[2].pImmutableSamplers = nullptr;

            bindings[3].binding = 3u;
            bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindings[3].descriptorCount = 1;
            bindings[3].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[3].pImmutableSamplers = nullptr;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<u32>(bindings.size());
            layoutInfo.pBindings = bindings.data();

            if (vkCreateDescriptorSetLayout(mvGetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayouts[1]) != VK_SUCCESS)
                throw std::runtime_error("failed to create descriptor set layout!");

            mvRegisterDescriptorSetLayoutAsset(&am, descriptorSetLayouts[1], "phong");
        }


        mvAssetID pipelineLayout = mvGetPipelineLayoutAsset(&am, descriptorSetLayouts);

    }

    //-----------------------------------------------------------------------------
    // allocate descriptor sets
    //-----------------------------------------------------------------------------
    std::vector<VkDescriptorSetLayout> layouts(GContext->graphics.swapChainImages.size(), 
        am.descriptorSetLayouts[mvGetDescriptorSetLayoutAsset(&am, "scene")].layout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = GContext->graphics.descriptorPool;
    allocInfo.descriptorSetCount = GContext->graphics.swapChainImages.size();
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(mvGetLogicalDevice(), &allocInfo, scene.descriptorSets) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate descriptor sets!");

    return scene;
}

void
mvUpdateSceneDescriptors(mvAssetManager& am, mvScene& scene, mvPointLight& light, mvDirectionLight& dlight)
{
    //-----------------------------------------------------------------------------
    // update descriptor sets
    //-----------------------------------------------------------------------------

    for (int i = 0; i < GContext->graphics.swapChainImages.size(); i++)
    {
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        descriptorWrites.resize(3);

        VkDescriptorBufferInfo sceneInfo;
        sceneInfo.buffer = am.dynBuffers[scene.sceneBuffer.buffers[i]].buffer.buffer;
        sceneInfo.offset = 0;
        sceneInfo.range = mvGetRequiredUniformBufferSize(sizeof(mvSceneData));

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = scene.descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &sceneInfo;

        VkDescriptorBufferInfo lightInfo;
        lightInfo.buffer = am.dynBuffers[light.buffer.buffers[i]].buffer.buffer;
        lightInfo.offset = 0;
        lightInfo.range = mvGetRequiredUniformBufferSize(sizeof(mvPointLightInfo));

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = scene.descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &lightInfo;

        VkDescriptorBufferInfo dlightInfo;
        dlightInfo.buffer = am.dynBuffers[dlight.buffer.buffers[i]].buffer.buffer;
        dlightInfo.offset = 0;
        dlightInfo.range = mvGetRequiredUniformBufferSize(sizeof(mvDirectionLightInfo));

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = scene.descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &dlightInfo;

        vkUpdateDescriptorSets(mvGetLogicalDevice(),
            static_cast<u32>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);

    }

}

void
mvBindScene(mvAssetManager& am, mvAssetID sceneId)
{
    mvScene& scene = am.scenes[sceneId].scene;
    mvUpdateBuffer(am.dynBuffers[scene.sceneBuffer.buffers[GContext->graphics.currentImageIndex]].buffer, &scene.data);

    mvAssetID layoutID = mvGetPipelineLayoutAsset(&am, "layout_2");
    VkPipelineLayout layout = am.pipelineLayouts[layoutID].layout.pipelineLayout;
    vkCmdBindDescriptorSets(mvGetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
        layout,
        0, 1, &scene.descriptorSets[GContext->graphics.currentImageIndex], 0, nullptr);
}