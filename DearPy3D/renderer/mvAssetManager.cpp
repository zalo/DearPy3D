#include "mvAssetManager.h"

void 
mvInitializeAssetManager(mvAssetManager* manager)
{
	manager->textures = new mvTextureAsset[manager->maxTextureCount];
	manager->buffers = new mvBufferAsset[manager->maxBufferCount];
	manager->dynBuffers = new mvBufferAsset[manager->maxDynBufferCount];
	manager->meshes = new mvMeshAsset[manager->maxMeshCount];
	manager->phongMaterials = new mvPhongMaterialAsset[manager->maxPhongMaterialCount];
	manager->samplers = new mvSamplerAsset[manager->maxSamplerCount];
	manager->pipelines = new mvPipelineAsset[manager->maxPipelineCount];
}

void 
mvPrepareResizeAssetManager(mvAssetManager* manager)
{
	vkDeviceWaitIdle(mvGetLogicalDevice());

	// cleanup samplers
	for (int i = 0; i < manager->samplerCount; i++)
	{
		vkDestroySampler(mvGetLogicalDevice(), manager->samplers[i].sampler.textureSampler, nullptr);
		manager->samplers[i].sampler.textureSampler = VK_NULL_HANDLE;
	}
	manager->samplerCount = 0u;

	// cleanup materials
	for (int i = 0; i < manager->phongMaterialCount; i++)
	{
		delete[] manager->phongMaterials[i].material.descriptorSets;
	}
	manager->phongMaterialCount = 0u;

	// cleanup buffers
	for (int i = 0; i < manager->bufferCount; i++)
	{
		vkDestroyBuffer(mvGetLogicalDevice(), manager->buffers[i].buffer.buffer, nullptr);
		mvFree(manager->buffers[i].buffer.memoryAllocation);

		manager->buffers[i].buffer.buffer = VK_NULL_HANDLE;
		manager->buffers[i].buffer.memoryAllocation = VK_NULL_HANDLE;
		manager->buffers[i].buffer.count = 0u;
	}
	manager->bufferCount = 0u;

	// cleanup dynamic buffers
	for (int i = 0; i < manager->dynBufferCount; i++)
	{
		vkDestroyBuffer(mvGetLogicalDevice(), manager->dynBuffers[i].buffer.buffer, nullptr);
		mvFree(manager->dynBuffers[i].buffer.memoryAllocation);

		manager->dynBuffers[i].buffer.buffer = VK_NULL_HANDLE;
		manager->dynBuffers[i].buffer.memoryAllocation = VK_NULL_HANDLE;
		manager->dynBuffers[i].buffer.count = 0u;
	}
	manager->dynBufferCount = 0u;

	// cleanup pipelines
	manager->pipelineCount = 0u;

}

void 
mvCleanupAssetManager(mvAssetManager* manager)
{
	vkDeviceWaitIdle(mvGetLogicalDevice());

	// cleanup samplers
	for (int i = 0; i < manager->samplerCount; i++)
	{
		vkDestroySampler(mvGetLogicalDevice(), manager->samplers[i].sampler.textureSampler, nullptr);
		manager->samplers[i].sampler.textureSampler = VK_NULL_HANDLE;
	}
	manager->samplerCount = 0u;

	// cleanup materials
	for (int i = 0; i < manager->phongMaterialCount; i++)
	{
		delete[] manager->phongMaterials[i].material.descriptorSets;
	}
	manager->phongMaterialCount = 0u;

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
		vkDestroyImageView(mvGetLogicalDevice(), manager->textures[i].texture.textureImageView, nullptr);
		vkDestroyImage(mvGetLogicalDevice(), manager->textures[i].texture.textureImage, nullptr);
		vkFreeMemory(mvGetLogicalDevice(), manager->textures[i].texture.textureImageMemory, nullptr);
	}

	delete[] manager->textures;
	delete[] manager->buffers;
	delete[] manager->dynBuffers;
	delete[] manager->meshes;
	delete[] manager->samplers;
	delete[] manager->pipelines;

	manager->samplers = nullptr;
	manager->textures = nullptr;
	manager->buffers = nullptr;
	manager->dynBuffers = nullptr;
	manager->meshes = nullptr;
	manager->phongMaterials = nullptr;
	manager->pipelines = nullptr;

	manager->samplerCount = 0u;
	manager->textureCount = 0u;
	manager->bufferCount = 0u;
	manager->dynBufferCount = 0u;
	manager->meshCount = 0u;
	manager->phongMaterialCount = 0u;
	manager->pipelineCount = 0u;
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
mvGetPhongMaterialAsset(mvAssetManager* manager, mvMaterialData materialData, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, const char* vertexShader, const char* pixelShader)
{
	mv_local_persist int temp = 0;
	temp++;
	std::string hash = std::string(pixelShader) + std::string(vertexShader) + std::to_string(temp);

	for (s32 i = 0; i < manager->phongMaterialCount; i++)
	{
		if (manager->phongMaterials[i].hash == hash)
			return i;
	}

	manager->phongMaterials[manager->phongMaterialCount].hash = hash;
	manager->phongMaterials[manager->phongMaterialCount].material = mvCreateMaterial(*manager, materialData, descriptorSetLayouts, vertexShader, pixelShader);
	manager->phongMaterialCount++;
	return manager->phongMaterialCount - 1;
}

mvAssetID 
mvGetSamplerAsset(mvAssetManager* manager)
{
	mv_local_persist std::string tempHash = "sampler";

	for (s32 i = 0; i < manager->samplerCount; i++)
	{
		if (manager->samplers[i].hash == tempHash)
			return i;
	}

	manager->samplers[manager->samplerCount].hash = tempHash;
	manager->samplers[manager->samplerCount].sampler = mvCreateSampler();
	manager->samplerCount++;
	return manager->samplerCount - 1;
}

mvAssetID 
mvGetPipelineAsset(mvAssetManager* manager, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, const char* vertexShader, const char* pixelShader)
{
	std::string hash = std::string(pixelShader) + std::string(vertexShader);
	for (s32 i = 0; i < manager->pipelineCount; i++)
	{
		if (manager->pipelines[i].hash == hash)
			return i;
	}

	manager->pipelines[manager->pipelineCount].hash = hash;
	mvFinalizePipeline(manager->pipelines[manager->pipelineCount].pipeline, descriptorSetLayouts, vertexShader, pixelShader);
	manager->pipelineCount++;
	return manager->pipelineCount - 1;
}