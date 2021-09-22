#include "mvMaterial.h"
#include <stdexcept>
#include "mvContext.h"
#include "mvTexture.h"
#include "mvSampler.h"

namespace DearPy3D {

	mvMaterial mvCreateMaterial(std::vector<mvMaterialData> materialData)
	{
		mvMaterial material{};
		auto blah = mvGetRequiredUniformBufferSize(sizeof(mvMaterialData));
		for (size_t i = 0; i < 3; i++)
			material.materialBuffer.buffers.push_back(mvCreateBuffer(
				materialData.data(), 
				materialData.size(), 
				mvGetRequiredUniformBufferSize(sizeof(mvMaterialData)), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT));

		auto vlayout = mvCreateVertexLayout(
			{
				mvVertexElementType::Position3D,
				mvVertexElementType::Normal,
				mvVertexElementType::Tangent,
				mvVertexElementType::Bitangent,
				mvVertexElementType::Texture2D 
			}
		);
		material.texture = mvCreateTexture("../../Resources/brickwall.jpg");
		material.sampler = mvCreateSampler();

		//-----------------------------------------------------------------------------
		// create descriptor set layout
		//-----------------------------------------------------------------------------
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		bindings.resize(2);

		bindings[0].binding = 0u;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[0].pImmutableSamplers = nullptr;

		bindings[1].binding = 1u;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		bindings[1].descriptorCount = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[1].pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		descriptorSetLayouts.resize(1);

		std::vector<VkDescriptorSet> descriptorSets;

		if (vkCreateDescriptorSetLayout(mvGetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayouts.back()) != VK_SUCCESS)
			throw std::runtime_error("failed to create descriptor set layout!");

		//-----------------------------------------------------------------------------
		// allocate descriptor sets
		//-----------------------------------------------------------------------------
		descriptorSets.resize(3);
		std::vector<VkDescriptorSetLayout> layouts(3, descriptorSetLayouts[0]);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = GContext->graphics.descriptorPool;
		allocInfo.descriptorSetCount = 3;
		allocInfo.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(mvGetLogicalDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate descriptor sets!");

		//-----------------------------------------------------------------------------
		// update descriptor sets
		//-----------------------------------------------------------------------------
		std::vector<VkWriteDescriptorSet> descriptorWrites;
		descriptorWrites.resize(3);
		for (int i = 0; i < 3; i++)
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = material.texture.textureImageView;
			imageInfo.sampler = material.sampler.textureSampler;

			descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[i].dstSet = descriptorSets[i];
			descriptorWrites[i].dstBinding = 0;
			descriptorWrites[i].dstArrayElement = 0;
			descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[i].descriptorCount = 1;
			descriptorWrites[i].pImageInfo = &imageInfo;

			VkDescriptorBufferInfo materialInfo;
			materialInfo.buffer = material.materialBuffer.buffers[i].buffer;
			materialInfo.offset = 0;
			materialInfo.range = mvGetRequiredUniformBufferSize(sizeof(mvMaterialData));

			descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[i].dstSet = descriptorSets[i];
			descriptorWrites[i].dstBinding = 1;
			descriptorWrites[i].dstArrayElement = 0;
			descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			descriptorWrites[i].descriptorCount = 1;
			descriptorWrites[i].pBufferInfo = &materialInfo;
		}

		vkUpdateDescriptorSets(mvGetLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

		material.pipeline.layout = vlayout;
		material.pipeline.vertexShader = mvCreateShader("../../DearPy3D/shaders/vs_shader.vert.spv");
		material.pipeline.fragShader = mvCreateShader("../../DearPy3D/shaders/ps_shader.frag.spv");
		material.pipeline.descriptorSetLayouts = descriptorSetLayouts;
		material.pipeline.descriptorSets = descriptorSets;
		mvFinalizePipeline(material.pipeline);

		return material;
	}

	void mvCleanupMaterial(mvMaterial& material)
	{
		vkDeviceWaitIdle(mvGetLogicalDevice());
		mvCleanupSampler(material.sampler);
		mvCleanupTexture(material.texture);
		for(auto& buffer : material.materialBuffer.buffers)
			mvCleanupBuffer(buffer);
	}

}