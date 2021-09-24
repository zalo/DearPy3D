#include "mvPointLight.h"
#include "mvContext.h"

namespace DearPy3D {

    mvPointLight mvCreatePointLight(glm::vec3 pos)
    {

        mvPointLight light;

        mvPointLightInfo info{};
        info.viewLightPos = glm::vec4(pos, 1.0f);

        for (size_t i = 0; i < 3; i++)
            light.buffer.buffers.push_back(mvCreateDynamicBuffer(
                &info, 
                1, 
                mvGetRequiredUniformBufferSize(sizeof(mvPointLightInfo)), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT));

        //-----------------------------------------------------------------------------
        // create descriptor set layout
        //-----------------------------------------------------------------------------
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        bindings.resize(1);

        bindings[0].binding = 0u;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[0].pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(mvGetLogicalDevice(), &layoutInfo, nullptr, &light.descriptorSetLayout) != VK_SUCCESS)
            throw std::runtime_error("failed to create descriptor set layout!");

        //-----------------------------------------------------------------------------
        // allocate descriptor sets
        //-----------------------------------------------------------------------------
        std::vector<VkDescriptorSetLayout> layouts(3, light.descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = GContext->graphics.descriptorPool;
        allocInfo.descriptorSetCount = 3;
        allocInfo.pSetLayouts = layouts.data();

        if (vkAllocateDescriptorSets(mvGetLogicalDevice(), &allocInfo, light.descriptorSets) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate descriptor sets!");

        //-----------------------------------------------------------------------------
        // update descriptor sets
        //-----------------------------------------------------------------------------

        for (int i = 0; i < 3; i++)
        {
            std::vector<VkWriteDescriptorSet> descriptorWrites;
            descriptorWrites.resize(1);

            VkDescriptorBufferInfo materialInfo;
            materialInfo.buffer = light.buffer.buffers[i].buffer;
            materialInfo.offset = 0;
            materialInfo.range = mvGetRequiredUniformBufferSize(sizeof(mvPointLightInfo));

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = light.descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &materialInfo;

            vkUpdateDescriptorSets(mvGetLogicalDevice(),
                static_cast<uint32_t>(descriptorWrites.size()),
                descriptorWrites.data(), 0, nullptr);

        }

        return light;
    }

    void mvBind(mvPointLight& light, VkPipelineLayout pipelineLayout)
    {
        vkCmdBindDescriptorSets(mvGetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout, 1, 1, &light.descriptorSets[GContext->graphics.currentImageIndex], 0, nullptr);
    }

    void mvCleanupPointLight(mvPointLight& light)
    {
        for (auto& item : light.buffer.buffers)
            mvCleanupBuffer(item);
    }
}
