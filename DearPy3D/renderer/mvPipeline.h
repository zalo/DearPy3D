#pragma once

#include <memory>
#include <vulkan/vulkan.h>
#include "mvShader.h"
#include "mvVertexLayout.h"

namespace DearPy3D {

	struct mvPipeline
	{
		mvShader                           vertexShader;
		mvShader                           fragShader;
		mvVertexLayout                     layout;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		std::vector<VkDescriptorSet>       descriptorSets;
		VkPipelineLayout                   pipelineLayout = VK_NULL_HANDLE;
		VkPipeline                         pipeline = VK_NULL_HANDLE;
		uint32_t                           _index = 0u; // uniform offset index
	};

	void mvFinalizePipeline(mvPipeline& pipeline);
	void mvBind            (mvPipeline& pipeline);

}