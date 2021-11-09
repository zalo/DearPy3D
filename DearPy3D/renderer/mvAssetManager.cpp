#include "mvAssetManager.h"
#include <imgui.h>

void 
mvInitializeAssetManager(mvAssetManager* manager)
{
	manager->textures = new mvTextureAsset[manager->maxTextureCount];
	manager->buffers = new mvBufferAsset[manager->maxBufferCount];
	manager->dynBuffers = new mvBufferAsset[manager->maxDynBufferCount];
	manager->meshes = new mvMeshAsset[manager->maxMeshCount];
	manager->phongMaterials = new mvPhongMaterialAsset[manager->maxPhongMaterialCount];
	manager->pipelines = new mvPipelineAsset[manager->maxPipelineCount];
	manager->pipelineLayouts = new mvPipelineLayoutAsset[manager->maxPipelineLayoutCount];
	manager->scenes = new mvSceneAsset[manager->maxSceneCount];
	manager->descriptorSetLayouts = new mvDescriptorSetLayoutAsset[manager->maxDescriptorSetLayoutCount];
	manager->nodes = new mvNodeAsset[manager->maxNodeCount];
}

void
mvPrepareResizeAssetManager(mvAssetManager* manager)
{
	vkDeviceWaitIdle(mvGetLogicalDevice());

	// cleanup pipelines
	for (int i = 0; i < manager->pipelineCount; i++)
	{
		vkDestroyPipeline(mvGetLogicalDevice(), manager->pipelines[i].pipeline.pipeline, nullptr);
	}
	manager->pipelineCount = 0u;

	for (int i = 0; i < manager->phongMaterialCount; i++)
	{
		mvPipelineSpec spec{};
		spec.backfaceCulling = true;
		spec.depthTest = true;
		spec.wireFrame = false;
		spec.depthWrite = true;
		spec.vertexShader = manager->phongMaterials[i].material.vertexShader;
		spec.pixelShader = manager->phongMaterials[i].material.pixelShader;
		spec.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		manager->phongMaterials[i].material.pipeline = mvGetPipelineAsset(manager, spec);
	}

}

void 
mvCleanupAssetManager(mvAssetManager* manager)
{
	vkDeviceWaitIdle(mvGetLogicalDevice());

	// cleanup materials
	for (int i = 0; i < manager->phongMaterialCount; i++)
	{
		delete[] manager->phongMaterials[i].material.descriptorSets;
	}
	manager->phongMaterialCount = 0u;

	// cleanup scene
	for (int i = 0; i < manager->sceneCount; i++)
	{
		delete[] manager->scenes[i].scene.descriptorSets;
	}
	manager->sceneCount = 0u;

	// buffers
	for (int i = 0; i < manager->bufferCount; i++)
	{
		vkDestroyBuffer(mvGetLogicalDevice(), manager->buffers[i].buffer.buffer, nullptr);
		mvFree(manager->buffers[i].buffer.memoryAllocation);

		manager->buffers[i].buffer.buffer = VK_NULL_HANDLE;
		manager->buffers[i].buffer.memoryAllocation = VK_NULL_HANDLE;
		manager->buffers[i].buffer.count = 0u;
	}

	// dynamic buffers
	for (int i = 0; i < manager->dynBufferCount; i++)
	{
		vkDestroyBuffer(mvGetLogicalDevice(), manager->dynBuffers[i].buffer.buffer, nullptr);
		mvFree(manager->dynBuffers[i].buffer.memoryAllocation);

		manager->dynBuffers[i].buffer.buffer = VK_NULL_HANDLE;
		manager->dynBuffers[i].buffer.memoryAllocation = VK_NULL_HANDLE;
		manager->dynBuffers[i].buffer.count = 0u;
	}

	// textures
	for (int i = 0; i < manager->textureCount; i++)
	{
		vkDestroySampler(mvGetLogicalDevice(), manager->textures[i].texture.imageInfo.sampler, nullptr);
		vkDestroyImageView(mvGetLogicalDevice(), manager->textures[i].texture.imageInfo.imageView, nullptr);
		vkDestroyImage(mvGetLogicalDevice(), manager->textures[i].texture.textureImage, nullptr);
		vkFreeMemory(mvGetLogicalDevice(), manager->textures[i].texture.textureImageMemory, nullptr);
	}

	// descriptor set layouts
	for (int i = 0; i < manager->descriptorSetLayoutCount; i++)
	{
		vkDestroyDescriptorSetLayout(mvGetLogicalDevice(), manager->descriptorSetLayouts[i].layout, nullptr);
	}

	// pipeline layouts
	for (int i = 0; i < manager->pipelineLayoutCount; i++)
	{
		vkDestroyPipelineLayout(mvGetLogicalDevice(), manager->pipelineLayouts[i].layout.pipelineLayout, nullptr);
	}

	// cleanup pipelines
	for (int i = 0; i < manager->pipelineCount; i++)
	{
		vkDestroyPipeline(mvGetLogicalDevice(), manager->pipelines[i].pipeline.pipeline, nullptr);
	}
	manager->pipelineCount = 0u;

	delete[] manager->textures;
	delete[] manager->buffers;
	delete[] manager->dynBuffers;
	delete[] manager->meshes;
	delete[] manager->pipelines;
	delete[] manager->pipelineLayouts;
	delete[] manager->scenes;
	delete[] manager->descriptorSetLayouts;
	delete[] manager->nodes;

	manager->textures = nullptr;
	manager->buffers = nullptr;
	manager->dynBuffers = nullptr;
	manager->meshes = nullptr;
	manager->phongMaterials = nullptr;
	manager->pipelines = nullptr;
	manager->pipelineLayouts = nullptr;
	manager->scenes = nullptr;
	manager->descriptorSetLayouts = nullptr;
	manager->nodes = nullptr;

	manager->textureCount = 0u;
	manager->bufferCount = 0u;
	manager->dynBufferCount = 0u;
	manager->meshCount = 0u;
	manager->phongMaterialCount = 0u;
	manager->pipelineCount = 0u;
	manager->pipelineLayoutCount = 0u;
	manager->sceneCount = 0u;
	manager->nodeCount = 0u;
	manager->descriptorSetLayoutCount = 0u;
}

mvAssetID
mvGetTextureAsset(mvAssetManager* manager, const std::string& path)
{
	for (s32 i = 0; i < manager->textureCount; i++)
	{
		if (manager->textures[i].hash == path)
			return i;
	}

	manager->textures[manager->textureCount].hash = path;
	manager->textures[manager->textureCount].texture = mvCreateTexture(path);
	manager->textureCount++;
	return manager->textureCount - 1;
}

mvAssetID
mvGetBufferAsset(mvAssetManager* manager, void* data, u64 count, u64 size, VkBufferUsageFlags flags, const std::string& tag)
{
	for (s32 i = 0; i < manager->bufferCount; i++)
	{
		if (manager->buffers[i].hash == tag)
			return i;
	}

	manager->buffers[manager->bufferCount].hash = tag;
	manager->buffers[manager->bufferCount].buffer = mvCreateBuffer(data, count, size, flags);
	manager->bufferCount++;
	return manager->bufferCount - 1;
}

mvAssetID
mvGetDynamicBufferAsset(mvAssetManager* manager, void* data, u64 count, u64 size, VkBufferUsageFlags flags, const std::string& tag)
{
	for (s32 i = 0; i < manager->dynBufferCount; i++)
	{
		if (manager->dynBuffers[i].hash == tag)
			return i;
	}

	manager->dynBuffers[manager->dynBufferCount].hash = tag;
	manager->dynBuffers[manager->dynBufferCount].buffer = mvCreateDynamicBuffer(data, count, size, flags);
	manager->dynBufferCount++;
	return manager->dynBufferCount - 1;
}

mvAssetID
mvRegistryMeshAsset(mvAssetManager* manager, mvMesh mesh)
{
	manager->meshes[manager->meshCount].mesh = mesh;
	manager->meshCount++;
	return manager->meshCount - 1;
}

mvAssetID
mvGetPhongMaterialAsset(mvAssetManager* manager, mvMaterialData materialData, const char* vertexShader, const char* pixelShader)
{
	std::string hash = mvCreateHash(materialData);

	for (s32 i = 0; i < manager->phongMaterialCount; i++)
	{
		if (manager->phongMaterials[i].hash == hash)
			return i;
	}

	manager->phongMaterials[manager->phongMaterialCount].hash = hash;
	manager->phongMaterials[manager->phongMaterialCount].material = mvCreateMaterial(*manager, materialData, vertexShader, pixelShader);
	manager->phongMaterialCount++;
	return manager->phongMaterialCount - 1;
}

mvAssetID 
mvGetPipelineAsset(mvAssetManager* manager, mvPipelineSpec& spec)
{
	std::string hash =
		std::string(spec.pixelShader)
		+ std::string(spec.vertexShader)
		+ std::string(spec.backfaceCulling ? "T" : "F")
		+ std::string(spec.wireFrame ? "T" : "F")
		+ std::string(spec.depthWrite ? "T" : "F")
		+ std::string(spec.depthTest ? "T" : "F");

	for (s32 i = 0; i < manager->pipelineCount; i++)
	{
		if (manager->pipelines[i].hash == hash)
			return i;
	}

	manager->pipelines[manager->pipelineCount].hash = hash;
	manager->pipelines[manager->pipelineCount].pipeline = mvCreatePipeline(*manager, spec);
	manager->pipelineCount++;
	return manager->pipelineCount - 1;
}

mvAssetID 
mvGetSceneAsset(mvAssetManager* manager, mvSceneData sceneData)
{
	std::string hash = "scene_" + std::string(sceneData.doLighting ? "T" : "F");

	for (s32 i = 0; i < manager->sceneCount; i++)
	{
		if (manager->scenes[i].hash == hash)
			return i;
	}

	manager->scenes[manager->sceneCount].hash = hash;
	manager->scenes[manager->sceneCount].scene = mvCreateScene(*manager, sceneData);
	manager->sceneCount++;
	return manager->sceneCount - 1;
}

mvAssetID 
mvGetPipelineLayoutAsset(mvAssetManager* manager, std::vector<VkDescriptorSetLayout> descriptorSetLayouts)
{
	std::string hash = "layout_" + std::to_string(descriptorSetLayouts.size());

	for (s32 i = 0; i < manager->pipelineLayoutCount; i++)
	{
		if (manager->pipelineLayouts[i].hash == hash)
			return i;
	}

	manager->pipelineLayouts[manager->pipelineLayoutCount].hash = hash;
	manager->pipelineLayouts[manager->pipelineLayoutCount].layout = mvCreatePipelineLayout(descriptorSetLayouts);
	manager->pipelineLayoutCount++;
	return manager->pipelineLayoutCount - 1;
}

mvAssetID
mvGetPipelineLayoutAsset(mvAssetManager* manager, const std::string& tag)
{

	for (s32 i = 0; i < manager->pipelineLayoutCount; i++)
	{
		if (manager->pipelineLayouts[i].hash == tag)
			return i;
	}
	return -1;
}

mvAssetID
mvRegisterDescriptorSetLayoutAsset(mvAssetManager* manager, VkDescriptorSetLayout layout, const std::string& tag)
{
	manager->descriptorSetLayouts[manager->descriptorSetLayoutCount].hash = tag;
	manager->descriptorSetLayouts[manager->descriptorSetLayoutCount].layout = layout;
	manager->descriptorSetLayoutCount++;
	return manager->descriptorSetLayoutCount - 1;
}

mvAssetID 
mvGetDescriptorSetLayoutAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->descriptorSetLayoutCount; i++)
	{
		if (manager->descriptorSetLayouts[i].hash == tag)
			return i;
	}

	return -1;
}

mvAssetID
mvRegistryNodeAsset(mvAssetManager* manager, mvNode node)
{
	manager->nodes[manager->nodeCount].node = node;
	manager->nodeCount++;
	return manager->nodeCount - 1;
}

mvAssetID
mvRegistrySceneAsset(mvAssetManager* manager, mvScene scene)
{
	manager->scenes[manager->sceneCount].scene = scene;
	manager->sceneCount++;
	return manager->sceneCount - 1;
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
				ImGui::Text("Texture: %s", assetManager.textures[i].texture.file.c_str());
				ImGui::Image(assetManager.textures[i].texture.imguiID, ImVec2(50, 50));
			}
			ImGui::TreePop();
		}
	}
	ImGui::End();
}

std::string
mvCreateHash(mvMaterialData materialData)
{
	std::string hash =
		std::string(materialData.useTextureMap ? "T" : "F")
		+ std::string(materialData.useNormalMap ? "T" : "F")
		+ std::string(materialData.useSpecularMap ? "T" : "F")
		+ std::string(materialData.useGlossAlpha ? "T" : "F")
		+ std::string(materialData.hasAlpha ? "T" : "F")
		+ std::string(materialData.doLighting ? "T" : "F");

	// temporary until we have a "bindless" solution
	// to descriptor set updates
	mv_local_persist int i = 0;
	i++;
	hash.append(std::to_string(i));

	return hash;
}