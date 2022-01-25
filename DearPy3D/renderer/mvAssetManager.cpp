#include "mvAssetManager.h"
#include <imgui.h>
#include "mvGraphics.h"

void 
mvInitializeAssetManager(mvAssetManager* manager)
{
	manager->textures = new mvTextureAsset[manager->maxTextureCount];
	manager->buffers = new mvBufferAsset[manager->maxBufferCount];
	manager->meshes = new mvMeshAsset[manager->maxMeshCount];
	manager->scenes = new mvSceneAsset[manager->maxSceneCount];
	manager->nodes = new mvNodeAsset[manager->maxNodeCount];
}

void 
mvCleanupAssetManager(mvGraphics& graphics, mvAssetManager* manager)
{
	vkDeviceWaitIdle(graphics.logicalDevice);

	// cleanup scene
	for (int i = 0; i < manager->sceneCount; i++)
	{
		//delete[] manager->scenes[i].asset.descriptorSets.descriptorSets;
	}
	manager->sceneCount = 0u;

	// buffers
	for (int i = 0; i < manager->bufferCount; i++)
	{
		vkDestroyBuffer(graphics.logicalDevice, manager->buffers[i].asset.buffer, nullptr);
		vkFreeMemory(graphics.logicalDevice, manager->buffers[i].asset.deviceMemory, nullptr);

		manager->buffers[i].asset.buffer = VK_NULL_HANDLE;
		manager->buffers[i].asset.deviceMemory = VK_NULL_HANDLE;
		manager->buffers[i].asset.specification.count = 0u;
		manager->buffers[i].asset.specification.aligned = false;
	}

	// textures
	for (int i = 0; i < manager->textureCount; i++)
	{
		vkDestroySampler(graphics.logicalDevice, manager->textures[i].asset.imageInfo.sampler, nullptr);
		vkDestroyImageView(graphics.logicalDevice, manager->textures[i].asset.imageInfo.imageView, nullptr);
		vkDestroyImage(graphics.logicalDevice, manager->textures[i].asset.textureImage, nullptr);
		vkFreeMemory(graphics.logicalDevice, manager->textures[i].asset.textureImageMemory, nullptr);
	}

	for (int i = 0; i < manager->meshCount; i++)
	{
		vkDestroyBuffer(graphics.logicalDevice, manager->meshes[i].asset.indexBuffer.buffer, nullptr);
		vkDestroyBuffer(graphics.logicalDevice, manager->meshes[i].asset.vertexBuffer.buffer, nullptr);
		vkFreeMemory(graphics.logicalDevice, manager->meshes[i].asset.indexBuffer.deviceMemory, nullptr);
		vkFreeMemory(graphics.logicalDevice, manager->meshes[i].asset.vertexBuffer.deviceMemory, nullptr);

		manager->meshes[i].asset.indexBuffer.buffer = VK_NULL_HANDLE;
		manager->meshes[i].asset.indexBuffer.deviceMemory = VK_NULL_HANDLE;
		manager->meshes[i].asset.indexBuffer.specification.count = 0u;
		manager->meshes[i].asset.indexBuffer.specification.aligned = false;
		manager->meshes[i].asset.vertexBuffer.buffer = VK_NULL_HANDLE;
		manager->meshes[i].asset.vertexBuffer.deviceMemory = VK_NULL_HANDLE;
		manager->meshes[i].asset.vertexBuffer.specification.count = 0u;
		manager->meshes[i].asset.vertexBuffer.specification.aligned = false;
	}



	delete[] manager->textures;
	delete[] manager->buffers;
	delete[] manager->meshes;
	delete[] manager->scenes;
	delete[] manager->nodes;

	manager->textures = nullptr;
	manager->buffers = nullptr;
	manager->meshes = nullptr;
	manager->scenes = nullptr;
	manager->nodes = nullptr;

	manager->textureCount = 0u;
	manager->bufferCount = 0u;
	manager->meshCount = 0u;
	manager->sceneCount = 0u;
	manager->nodeCount = 0u;
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
mvGetTextureAssetID(mvGraphics& graphics, mvAssetManager* manager, const std::string& path)
{
	for (s32 i = 0; i < manager->textureCount; i++)
	{
		if (manager->textures[i].hash == path)
			return i;
	}

	manager->textures[manager->textureCount].hash = path;
	manager->textures[manager->textureCount].asset = create_texture(graphics, path.c_str());
	manager->textureCount++;
	return manager->textureCount - 1;
}

mvAssetID
mvGetTextureAssetID2(mvGraphics& graphics, mvAssetManager* manager, const std::string& path)
{
	for (s32 i = 0; i < manager->textureCount; i++)
	{
		if (manager->textures[i].hash == path)
			return i;
	}

	manager->textures[manager->textureCount].hash = path;
	manager->textures[manager->textureCount].asset = create_texture_cube(graphics, path.c_str());
	manager->textureCount++;
	return manager->textureCount - 1;
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
