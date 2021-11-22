#pragma once

void
preload_descriptorset_layouts(mvAssetManager& am)
{
	// create descriptor set layouts
	VkDescriptorSetLayout descriptorSetLayouts[4];

	mvDescriptorSetLayout globalLayout = mvCreateDescriptorSetLayout(
		{
			mvCreateDynamicUniformBufferDescriptorSpec(0u),
			mvCreateDynamicUniformBufferDescriptorSpec(1u),
			mvCreateDynamicUniformBufferDescriptorSpec(2u),
			mvCreateTextureDescriptorSpec(3u)
		});

	mvDescriptorSetLayout materialLayout = mvCreateDescriptorSetLayout(
		{
			mvCreateTextureDescriptorSpec(0u),
			mvCreateTextureDescriptorSpec(1u),
			mvCreateTextureDescriptorSpec(2u),
			mvCreateUniformBufferDescriptorSpec(3u)
		});


	mvDescriptorSetLayout perDrawLayout = mvCreateDescriptorSetLayout(
		{
			mvCreateDynamicUniformBufferDescriptorSpec(0u)
		});

	mvDescriptorSetLayout skyboxLayout = mvCreateDescriptorSetLayout(
		{
			mvCreateTextureDescriptorSpec(0u)
		});

	descriptorSetLayouts[0] = globalLayout.layout;
	descriptorSetLayouts[1] = materialLayout.layout;
	descriptorSetLayouts[2] = perDrawLayout.layout;
	descriptorSetLayouts[3] = skyboxLayout.layout;

	mvRegisterAsset(&am, "scene", descriptorSetLayouts[0]);
	mvRegisterAsset(&am, "phong", descriptorSetLayouts[1]);
	mvRegisterAsset(&am, "perdraw", descriptorSetLayouts[2]);
	mvRegisterAsset(&am, "skybox_pass", descriptorSetLayouts[3]);
}

void 
preload_pipeline_layouts(mvAssetManager& am)
{
	VkDescriptorSetLayout descriptorSetLayouts[4];

	descriptorSetLayouts[0] = mvGetRawDescriptorSetLayoutAsset(&am, "scene");
	descriptorSetLayouts[1] = mvGetRawDescriptorSetLayoutAsset(&am, "phong");
	descriptorSetLayouts[2] = mvGetRawDescriptorSetLayoutAsset(&am, "perdraw");
	descriptorSetLayouts[3] = mvGetRawDescriptorSetLayoutAsset(&am, "skybox_pass");

	{
		VkPipelineLayout pipelineLayout;
		VkPushConstantRange push_constant;
		push_constant.offset = 0;
		push_constant.size = 192;
		push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 3;
		pipelineLayoutInfo.pPushConstantRanges = &push_constant;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;

		MV_VULKAN(vkCreatePipelineLayout(mvGetLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout));

		mvAssetID pipelineLayoutID = mvRegisterAsset(&am, "main_pass", pipelineLayout);

		mvTransforms* transforms = new mvTransforms[am.maxMeshCount * 3];
		mvDescriptorSet descriptorSet = mvCreateDescriptorSet(am, descriptorSetLayouts[2], pipelineLayoutID);
		descriptorSet.descriptors.push_back(mvCreateDynamicUniformBufferDescriptor(am, mvCreateDynamicUniformBufferDescriptorSpec(0u), "transforms", am.maxMeshCount * 3, sizeof(mvTransforms), transforms));
		mvRegisterAsset(&am, "perdraw", descriptorSet);
		delete[] transforms;
	}

	{
		VkPipelineLayout pipelineLayout;
		VkPushConstantRange push_constant;
		push_constant.offset = 0;
		push_constant.size = 192;
		push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = &push_constant;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pSetLayouts = nullptr;

		MV_VULKAN(vkCreatePipelineLayout(mvGetLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout));

		mvAssetID pipelineLayoutID = mvRegisterAsset(&am, "shadow_pass", pipelineLayout);
	}

	{
		VkPipelineLayout pipelineLayout;
		VkPushConstantRange push_constant;
		push_constant.offset = 0;
		push_constant.size = 64;
		push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &push_constant;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayouts[3];

		MV_VULKAN(vkCreatePipelineLayout(mvGetLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout));

		mvAssetID pipelineLayoutID = mvRegisterAsset(&am, "skybox_pass", pipelineLayout);
	}
}

void
preload_pipelines(mvAssetManager& am)
{
	//-----------------------------------------------------------------------------
	// create pipeline
	//-----------------------------------------------------------------------------
	{
		mvPipelineSpec pipelineSpec{};
		pipelineSpec.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelineSpec.backfaceCulling = true;
		pipelineSpec.depthTest = true;
		pipelineSpec.depthWrite = true;
		pipelineSpec.wireFrame = false;
		pipelineSpec.vertexShader = "phong.vert.spv";
		pipelineSpec.pixelShader = "phong.frag.spv";
		pipelineSpec.width = (float)GContext->graphics.swapChainExtent.width;  // use viewport
		pipelineSpec.height = (float)GContext->graphics.swapChainExtent.height; // use viewport
		pipelineSpec.pipelineLayout = mvGetRawPipelineLayoutAsset(&am, "main_pass");
		pipelineSpec.renderPass = GContext->graphics.renderPass;
		pipelineSpec.layout = mvCreateVertexLayout(
			{
				mvVertexElementType::Position3D,
				mvVertexElementType::Normal,
				mvVertexElementType::Tangent,
				mvVertexElementType::Bitangent,
				mvVertexElementType::Texture2D
			});

		mvPipeline pipeline = mvCreatePipeline(am, pipelineSpec);
		mvRegisterAsset(&am, "main_pass", pipeline);
	}

	{
		//-----------------------------------------------------------------------------
		// create pipeline
		//-----------------------------------------------------------------------------
		mvPipelineSpec pipelineSpec{};
		pipelineSpec.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelineSpec.backfaceCulling = false;
		pipelineSpec.depthTest = true;
		pipelineSpec.depthWrite = false;
		pipelineSpec.wireFrame = false;
		pipelineSpec.vertexShader = "skybox.vert.spv";
		pipelineSpec.pixelShader = "skybox.frag.spv";
		pipelineSpec.width = (float)GContext->graphics.swapChainExtent.width;  // use viewport
		pipelineSpec.height = (float)GContext->graphics.swapChainExtent.height; // use viewport
		pipelineSpec.renderPass = GContext->graphics.renderPass;
		pipelineSpec.pipelineLayout = mvGetRawPipelineLayoutAsset(&am, "skybox_pass");
		pipelineSpec.layout = mvCreateVertexLayout(
			{
				mvVertexElementType::Position3D
			});

		mvPipeline pipeline = mvCreatePipeline(am, pipelineSpec);
		mvRegisterAsset(&am, "skybox_pass", pipeline);
	}
}