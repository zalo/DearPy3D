#include "mvAssetManager.h"

void 
mvInitializeAssetManager(mvAssetManager* manager)
{
	manager->textures = new mvTextureAsset[manager->maxTextureCount];
	manager->buffers = new mvBufferAsset[manager->maxBufferCount];
	manager->dynBuffers = new mvBufferAsset[manager->maxDynBufferCount];
	manager->meshes = new mvMeshAsset[manager->maxMeshCount];
}

void 
mvPrepareResizeAssetManager(mvAssetManager* manager)
{
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

}

void 
mvCleanupAssetManager(mvAssetManager* manager)
{

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

	delete[] manager->textures;
	delete[] manager->buffers;
	delete[] manager->dynBuffers;
	delete[] manager->meshes;

	manager->textures = nullptr;
	manager->buffers = nullptr;
	manager->dynBuffers = nullptr;
	manager->meshes = nullptr;

	manager->textureCount = 0u;
	manager->bufferCount = 0u;
	manager->dynBufferCount = 0u;
	manager->meshCount = 0u;
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