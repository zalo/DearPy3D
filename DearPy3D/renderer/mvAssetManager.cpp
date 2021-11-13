#include "mvAssetManager.h"
#include <imgui.h>

mv_internal void
mvPreloadAssetManager(mvAssetManager& am)
{

	// only need to recreate the pipeline
	if (mvGetDescriptorSetLayoutAssetID(&am, "scene") == MV_INVALID_ASSET_ID)
	{

		//-----------------------------------------------------------------------------
		// create descriptor set layout
		//-----------------------------------------------------------------------------

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

			mvRegisterAsset(&am, "scene", descriptorSetLayouts[0]);
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

			mvRegisterAsset(&am, "phong", descriptorSetLayouts[1]);
		}

		//-----------------------------------------------------------------------------
		// create pipeline layouts
		//-----------------------------------------------------------------------------

		//setup push constants
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
		pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
		pipelineLayoutInfo.pPushConstantRanges = &push_constant;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

		if (vkCreatePipelineLayout(mvGetLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create pipeline layout!");

		mvRegisterAsset(&am, "main_pass", pipelineLayout);
	}

	//-----------------------------------------------------------------------------
	// create pipeline
	//-----------------------------------------------------------------------------
	mvPipelineSpec pipelineSpec{};
	pipelineSpec.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineSpec.backfaceCulling = true;
	pipelineSpec.depthTest = true;
	pipelineSpec.depthWrite = true;
	pipelineSpec.wireFrame = false;
	pipelineSpec.vertexShader = "vs_shader.vert.spv";
	pipelineSpec.pixelShader = "ps_shader.frag.spv";
	pipelineSpec.width = (float)GContext->graphics.swapChainExtent.width;  // use viewport
	pipelineSpec.height = (float)GContext->graphics.swapChainExtent.height; // use viewport
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

void 
mvInitializeAssetManager(mvAssetManager* manager)
{
	manager->textures = new mvTextureAsset[manager->maxTextureCount];
	manager->buffers = new mvBufferAsset[manager->maxBufferCount];
	manager->meshes = new mvMeshAsset[manager->maxMeshCount];
	manager->phongMaterials = new mvPhongMaterialAsset[manager->maxPhongMaterialCount];
	manager->pipelines = new mvPipelineAsset[manager->maxPipelineCount];
	manager->pipelineLayouts = new mvPipelineLayoutAsset[manager->maxPipelineLayoutCount];
	manager->scenes = new mvSceneAsset[manager->maxSceneCount];
	manager->descriptorSetLayouts = new mvDescriptorSetLayoutAsset[manager->maxDescriptorSetLayoutCount];
	manager->nodes = new mvNodeAsset[manager->maxNodeCount];

	mvPreloadAssetManager(*manager);
}

void
mvPrepareResizeAssetManager(mvAssetManager* manager)
{
	vkDeviceWaitIdle(mvGetLogicalDevice());

	// cleanup pipelines
	for (int i = 0; i < manager->pipelineCount; i++)
	{
		vkDestroyPipeline(mvGetLogicalDevice(), manager->pipelines[i].asset.pipeline, nullptr);
	}
	manager->pipelineCount = 0u;

	mvPreloadAssetManager(*manager);

	for (int i = 0; i < manager->phongMaterialCount; i++)
	{
		mvPipelineSpec spec{};
		spec.backfaceCulling = true;
		spec.depthTest = true;
		spec.wireFrame = false;
		spec.depthWrite = true;
		spec.vertexShader = manager->phongMaterials[i].asset.vertexShader;
		spec.pixelShader = manager->phongMaterials[i].asset.pixelShader;
		spec.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		manager->phongMaterials[i].asset.pipeline = mvGetPipelineLayoutAssetID(manager, "main_pass");
	}

}

void 
mvCleanupAssetManager(mvAssetManager* manager)
{
	vkDeviceWaitIdle(mvGetLogicalDevice());

	// cleanup materials
	for (int i = 0; i < manager->phongMaterialCount; i++)
	{
		delete[] manager->phongMaterials[i].asset.descriptorSets;
	}
	manager->phongMaterialCount = 0u;

	// cleanup scene
	for (int i = 0; i < manager->sceneCount; i++)
	{
		delete[] manager->scenes[i].asset.descriptorSets;
	}
	manager->sceneCount = 0u;

	// buffers
	for (int i = 0; i < manager->bufferCount; i++)
	{
		vkDestroyBuffer(mvGetLogicalDevice(), manager->buffers[i].asset.buffer, nullptr);
		mvFree(manager->buffers[i].asset.memoryAllocation);

		manager->buffers[i].asset.buffer = VK_NULL_HANDLE;
		manager->buffers[i].asset.memoryAllocation = VK_NULL_HANDLE;
		manager->buffers[i].asset.count = 0u;
	}

	// textures
	for (int i = 0; i < manager->textureCount; i++)
	{
		vkDestroySampler(mvGetLogicalDevice(), manager->textures[i].asset.imageInfo.sampler, nullptr);
		vkDestroyImageView(mvGetLogicalDevice(), manager->textures[i].asset.imageInfo.imageView, nullptr);
		vkDestroyImage(mvGetLogicalDevice(), manager->textures[i].asset.textureImage, nullptr);
		vkFreeMemory(mvGetLogicalDevice(), manager->textures[i].asset.textureImageMemory, nullptr);
	}

	// descriptor set layouts
	for (int i = 0; i < manager->descriptorSetLayoutCount; i++)
	{
		vkDestroyDescriptorSetLayout(mvGetLogicalDevice(), manager->descriptorSetLayouts[i].asset, nullptr);
	}

	// pipeline layouts
	for (int i = 0; i < manager->pipelineLayoutCount; i++)
	{
		vkDestroyPipelineLayout(mvGetLogicalDevice(), manager->pipelineLayouts[i].asset, nullptr);
	}

	// cleanup pipelines
	for (int i = 0; i < manager->pipelineCount; i++)
	{
		vkDestroyPipeline(mvGetLogicalDevice(), manager->pipelines[i].asset.pipeline, nullptr);
	}
	manager->pipelineCount = 0u;

	delete[] manager->textures;
	delete[] manager->buffers;
	delete[] manager->meshes;
	delete[] manager->pipelines;
	delete[] manager->pipelineLayouts;
	delete[] manager->scenes;
	delete[] manager->descriptorSetLayouts;
	delete[] manager->nodes;

	manager->textures = nullptr;
	manager->buffers = nullptr;
	manager->meshes = nullptr;
	manager->phongMaterials = nullptr;
	manager->pipelines = nullptr;
	manager->pipelineLayouts = nullptr;
	manager->scenes = nullptr;
	manager->descriptorSetLayouts = nullptr;
	manager->nodes = nullptr;

	manager->textureCount = 0u;
	manager->bufferCount = 0u;
	manager->meshCount = 0u;
	manager->phongMaterialCount = 0u;
	manager->pipelineCount = 0u;
	manager->pipelineLayoutCount = 0u;
	manager->sceneCount = 0u;
	manager->nodeCount = 0u;
	manager->descriptorSetLayoutCount = 0u;
}

//-----------------------------------------------------------------------------
// pipelines
//-----------------------------------------------------------------------------

mvAssetID
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvPipeline asset)
{
	manager->pipelines[manager->pipelineCount].asset = asset;
	manager->pipelines[manager->pipelineCount].hash = tag;
	manager->pipelineCount++;
	return manager->pipelineCount - 1;
}

mvAssetID
mvGetPipelineAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->pipelineCount; i++)
	{
		if (manager->pipelines[i].hash == tag)
			return i;
	}

	return MV_INVALID_ASSET_ID;
}

mvPipeline*
mvGetRawPipelineAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->pipelineCount; i++)
	{
		if (manager->pipelines[i].hash == tag)
			return &manager->pipelines[i].asset;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// pipeline layouts
//-----------------------------------------------------------------------------

mvAssetID
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, VkPipelineLayout asset)
{
	manager->pipelineLayouts[manager->pipelineLayoutCount].asset = asset;
	manager->pipelineLayouts[manager->pipelineLayoutCount].hash = tag;
	manager->pipelineLayoutCount++;
	return manager->pipelineLayoutCount - 1;
}

mvAssetID
mvGetPipelineLayoutAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->pipelineLayoutCount; i++)
	{
		if (manager->pipelineLayouts[i].hash == tag)
			return i;
	}

	return MV_INVALID_ASSET_ID;
}

VkPipelineLayout
mvGetRawPipelineLayoutAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->pipelineLayoutCount; i++)
	{
		if (manager->pipelineLayouts[i].hash == tag)
			return manager->pipelineLayouts[i].asset;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// descriptor set layouts
//-----------------------------------------------------------------------------

mvAssetID
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, VkDescriptorSetLayout asset)
{
	manager->descriptorSetLayouts[manager->descriptorSetLayoutCount].hash = tag;
	manager->descriptorSetLayouts[manager->descriptorSetLayoutCount].asset = asset;
	manager->descriptorSetLayoutCount++;
	return manager->descriptorSetLayoutCount - 1;
}

mvAssetID
mvGetDescriptorSetLayoutAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->descriptorSetLayoutCount; i++)
	{
		if (manager->descriptorSetLayouts[i].hash == tag)
			return i;
	}

	return MV_INVALID_ASSET_ID;
}

VkDescriptorSetLayout
mvGetRawDescriptorSetLayoutAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->descriptorSetLayoutCount; i++)
	{
		if (manager->descriptorSetLayouts[i].hash == tag)
			return manager->descriptorSetLayouts[i].asset;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// scenes
//-----------------------------------------------------------------------------

mvAssetID
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvScene asset)
{
	manager->scenes[manager->sceneCount].asset = asset;
	manager->scenes[manager->sceneCount].hash = tag;
	manager->sceneCount++;
	return manager->sceneCount - 1;
}

mvAssetID
mvGetSceneAssetID(mvAssetManager* manager, const std::string& tag)
{

	for (s32 i = 0; i < manager->sceneCount; i++)
	{
		if (manager->scenes[i].hash == tag)
			return i;
	}

	return MV_INVALID_ASSET_ID;
}

mvScene*
mvGetRawSceneAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->sceneCount; i++)
	{
		if (manager->scenes[i].hash == tag)
			return &manager->scenes[i].asset;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// textures
//-----------------------------------------------------------------------------

mvAssetID 
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvTexture asset)
{
	manager->textures[manager->textureCount].asset = asset;
	manager->textures[manager->textureCount].hash = tag;
	manager->textureCount++;
	return manager->textureCount - 1;
}

mvAssetID
mvGetTextureAssetID(mvAssetManager* manager, const std::string& path)
{
	for (s32 i = 0; i < manager->textureCount; i++)
	{
		if (manager->textures[i].hash == path)
			return i;
	}

	manager->textures[manager->textureCount].hash = path;
	manager->textures[manager->textureCount].asset = mvCreateTexture(path);
	manager->textureCount++;
	return manager->textureCount - 1;
}

mvTexture*
mvGetRawTextureAsset(mvAssetManager* manager, const std::string& path)
{
	for (s32 i = 0; i < manager->textureCount; i++)
	{
		if (manager->textures[i].hash == path)
			return &manager->textures[i].asset;
	}

	manager->textures[manager->textureCount].hash = path;
	manager->textures[manager->textureCount].asset = mvCreateTexture(path);
	manager->textureCount++;
	return &manager->textures[manager->textureCount - 1].asset;
}

//-----------------------------------------------------------------------------
// meshes
//-----------------------------------------------------------------------------

mvAssetID
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvMesh asset)
{
	manager->meshes[manager->meshCount].asset = asset;
	manager->meshes[manager->meshCount].hash = tag;
	manager->meshCount++;
	return manager->meshCount - 1;
}

mvMesh*
mvGetRawMeshAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->meshCount; i++)
	{
		if (manager->meshes[i].hash == tag)
			return &manager->meshes[i].asset;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// uniform buffers
//-----------------------------------------------------------------------------

mvAssetID
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvBuffer asset)
{
	manager->buffers[manager->bufferCount].asset = asset;
	manager->buffers[manager->bufferCount].hash = tag;
	manager->bufferCount++;
	return manager->bufferCount - 1;
}

mvAssetID
mvGetBufferAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->bufferCount; i++)
	{
		if (manager->buffers[i].hash == tag)
			return i;
	}

	return MV_INVALID_ASSET_ID;
}

mvBuffer*
mvGetRawBufferAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->bufferCount; i++)
	{
		if (manager->buffers[i].hash == tag)
			return &manager->buffers[i].asset;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// materials
//-----------------------------------------------------------------------------

mvAssetID
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvMaterial asset)
{
	manager->phongMaterials[manager->phongMaterialCount].asset = asset;
	manager->phongMaterials[manager->phongMaterialCount].hash = tag;
	manager->phongMaterialCount++;
	return manager->phongMaterialCount - 1;
}

mvAssetID
mvGetMaterialAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->phongMaterialCount; i++)
	{
		if (manager->phongMaterials[i].hash == tag)
			return i;
	}

	return MV_INVALID_ASSET_ID;
}

mvMaterial*
mvGetRawMaterialAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->phongMaterialCount; i++)
	{
		if (manager->phongMaterials[i].hash == tag)
			return &manager->phongMaterials[i].asset;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// nodes
//-----------------------------------------------------------------------------

mvAssetID
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvNode asset)
{
	manager->nodes[manager->nodeCount].asset = asset;
	manager->nodes[manager->nodeCount].hash = tag;
	manager->nodeCount++;
	return manager->nodeCount - 1;
}

mvAssetID
mvGetNodeAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->nodeCount; i++)
	{
		if (manager->nodes[i].hash == tag)
			return i;
	}

	return MV_INVALID_ASSET_ID;
}

mvNode* 
mvGetRawNodeAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->nodeCount; i++)
	{
		if (manager->nodes[i].hash == tag)
			return &manager->nodes[i].asset;
	}

	return nullptr;
}

void 
mvShowAssetManager(mvAssetManager& assetManager)
{
	if (ImGui::Begin("Asset Manager", nullptr))
	{
		if (ImGui::TreeNodeEx("Textures", assetManager.textureCount == 0u ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None, "Textures: Count %d", assetManager.textureCount))
		{
			for (u32 i = 0; i < assetManager.textureCount; i++)
			{
				ImGui::Text("Texture: %s", assetManager.textures[i].asset.file.c_str());
				ImGui::Image(assetManager.textures[i].asset.imguiID, ImVec2(50, 50));
			}
			ImGui::TreePop();
		}
	}
	ImGui::End();
}
