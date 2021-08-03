#pragma once

#include <memory>
#include <vulkan/vulkan.h>
#include "mvShader.h"
#include "mvVertexLayout.h"
#include "mvDescriptorSet.h"

namespace Marvel {

	//---------------------------------------------------------------------
	// forward declarations
	//---------------------------------------------------------------------
	class mvGraphics;
	class mvCommandBuffer;

	//---------------------------------------------------------------------
	// mvPipeline
	//---------------------------------------------------------------------
	class mvPipeline
	{

	public:

		void bind                   (mvCommandBuffer& commandBuffer);
		void finalize               (mvGraphics& graphics);
		void setDescriptorSets       (std::vector<std::shared_ptr<mvDescriptorSet>>);
		void setDescriptorSetLayout (std::shared_ptr<mvDescriptorSetLayout>);
		void setVertexLayout        (mvVertexLayout);
		void setVertexShader        (mvGraphics& graphics, const std::string& file);
		void setFragmentShader      (mvGraphics& graphics, const std::string& file);
		void finish                 (mvGraphics&);

		VkPipelineLayout getLayout() { return _pipelineLayout; }

	private:

		std::unique_ptr<mvShader>              _vertexShader = nullptr;
		std::unique_ptr<mvShader>              _fragShader = nullptr;
		mvVertexLayout                         _layout;
		std::shared_ptr<mvDescriptorSetLayout> _descriptorSetLayout = nullptr;
		std::vector<std::shared_ptr<mvDescriptorSet>> _descriptorSets;
		VkPipelineLayout                       _pipelineLayout = nullptr;
		VkPipeline                             _pipeline = nullptr;

	};

}