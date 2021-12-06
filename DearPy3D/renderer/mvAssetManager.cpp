#include "mvAssetManager.h"
#include <imgui.h>

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
	manager->descriptorSets = new mvDescriptorSetAsset[manager->maxDescriptorSetCount];
	manager->nodes = new mvNodeAsset[manager->maxNodeCount];
	manager->renderPasses = new mvRenderPassAsset[manager->maxRenderPassCount];
	manager->frameBuffers = new mvFrameBufferAsset[manager->maxFrameBufferCount];
}

void 
mvCleanupAssetManager(mvAssetManager* manager)
{
	vkDeviceWaitIdle(mvGetLogicalDevice());

	// cleanup scene
	for (int i = 0; i < manager->sceneCount; i++)
	{
		//delete[] manager->scenes[i].asset.descriptorSets.descriptorSets;
	}
	manager->sceneCount = 0u;

	// cleanup descriptor sets
	for (int i = 0; i < manager->descriptorSetCount; i++)
	{
		mvDescriptorSet set = manager->descriptorSets[i].asset;
		delete[] manager->descriptorSets[i].asset.descriptorSets;
	}
	manager->descriptorSetCount = 0u;

	// buffers
	for (int i = 0; i < manager->bufferCount; i++)
	{
		vkDestroyBuffer(mvGetLogicalDevice(), manager->buffers[i].asset.buffer, nullptr);
		vkFreeMemory(mvGetLogicalDevice(), manager->buffers[i].asset.deviceMemory, nullptr);

		manager->buffers[i].asset.buffer = VK_NULL_HANDLE;
		manager->buffers[i].asset.deviceMemory = VK_NULL_HANDLE;
		manager->buffers[i].asset.specification.count = 0u;
		manager->buffers[i].asset.specification.aligned = false;
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

	// frame buffers
	for (int i = 0; i < manager->frameBufferCount; i++)
	{
		vkDestroyFramebuffer(mvGetLogicalDevice(), manager->frameBuffers[i].asset, nullptr);
	}

	// frame buffers
	for (int i = 0; i < manager->renderPassCount; i++)
	{
		vkDestroyRenderPass(mvGetLogicalDevice(), manager->renderPasses[i].asset, nullptr);
	}

	delete[] manager->textures;
	delete[] manager->buffers;
	delete[] manager->meshes;
	delete[] manager->pipelines;
	delete[] manager->pipelineLayouts;
	delete[] manager->scenes;
	delete[] manager->descriptorSetLayouts;
	delete[] manager->descriptorSets;
	delete[] manager->nodes;
	delete[] manager->renderPasses;

	manager->textures = nullptr;
	manager->buffers = nullptr;
	manager->meshes = nullptr;
	manager->phongMaterials = nullptr;
	manager->pipelines = nullptr;
	manager->pipelineLayouts = nullptr;
	manager->scenes = nullptr;
	manager->descriptorSetLayouts = nullptr;
	manager->descriptorSets = nullptr;
	manager->nodes = nullptr;
	manager->renderPasses = nullptr;

	manager->textureCount = 0u;
	manager->bufferCount = 0u;
	manager->meshCount = 0u;
	manager->phongMaterialCount = 0u;
	manager->pipelineCount = 0u;
	manager->pipelineLayoutCount = 0u;
	manager->sceneCount = 0u;
	manager->nodeCount = 0u;
	manager->descriptorSetLayoutCount = 0u;
	manager->descriptorSetCount = 0u;
	manager->renderPassCount = 0u;
}

//-----------------------------------------------------------------------------
// pipelines
//-----------------------------------------------------------------------------

mvAssetID 
mvResetPipeline(mvAssetManager* manager, const std::string& tag, mvPipeline asset)
{
	for (s32 i = 0; i < manager->pipelineCount; i++)
	{
		if (manager->pipelines[i].hash == tag)
		{
			vkDestroyPipeline(mvGetLogicalDevice(), manager->pipelines[i].asset.pipeline, nullptr);
			manager->pipelines[i].asset = asset;
			return i;
		}
	}

	manager->pipelines[manager->pipelineCount].asset = asset;
	manager->pipelines[manager->pipelineCount].hash = tag;
	manager->pipelineCount++;
	return manager->pipelineCount - 1;
}

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
// render passes
//-----------------------------------------------------------------------------

mvAssetID
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, VkRenderPass asset)
{
	manager->renderPasses[manager->renderPassCount].asset = asset;
	manager->renderPasses[manager->renderPassCount].hash = tag;
	manager->renderPassCount++;
	return manager->renderPassCount - 1;
}

mvAssetID
mvGetRenderPassAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->renderPassCount; i++)
	{
		if (manager->renderPasses[i].hash == tag)
			return i;
	}

	return MV_INVALID_ASSET_ID;
}

VkRenderPass
mvGetRawRenderPassAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->renderPassCount; i++)
	{
		if (manager->renderPasses[i].hash == tag)
			return manager->renderPasses[i].asset;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------
// frame buffers
//-----------------------------------------------------------------------------

mvAssetID
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, VkFramebuffer asset)
{
	manager->frameBuffers[manager->frameBufferCount].asset = asset;
	manager->frameBuffers[manager->frameBufferCount].hash = tag;
	manager->frameBufferCount++;
	return manager->frameBufferCount - 1;
}

mvAssetID
mvGetFrameBufferAssetID(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->frameBufferCount; i++)
	{
		if (manager->frameBuffers[i].hash == tag)
			return i;
	}

	return MV_INVALID_ASSET_ID;
}

VkFramebuffer
mvGetRawFrameBufferAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->frameBufferCount; i++)
	{
		if (manager->frameBuffers[i].hash == tag)
			return manager->frameBuffers[i].asset;
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
// descriptor sets
//-----------------------------------------------------------------------------

mvAssetID
mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvDescriptorSet asset)
{
	manager->descriptorSets[manager->descriptorSetCount].asset = asset;
	manager->descriptorSets[manager->descriptorSetCount].hash = tag;
	manager->descriptorSetCount++;
	return manager->descriptorSetCount - 1;
}

mvAssetID
mvGetDescriptorSetAssetID(mvAssetManager* manager, const std::string& tag)
{

	for (s32 i = 0; i < manager->descriptorSetCount; i++)
	{
		if (manager->descriptorSets[i].hash == tag)
			return i;
	}

	return MV_INVALID_ASSET_ID;
}

mvDescriptorSet*
mvGetRawDescriptorSetAsset(mvAssetManager* manager, const std::string& tag)
{
	for (s32 i = 0; i < manager->descriptorSetCount; i++)
	{
		if (manager->descriptorSets[i].hash == tag)
			return &manager->descriptorSets[i].asset;
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

mvAssetID
mvGetTextureAssetID2(mvAssetManager* manager, const std::string& path)
{
	for (s32 i = 0; i < manager->textureCount; i++)
	{
		if (manager->textures[i].hash == path)
			return i;
	}

	manager->textures[manager->textureCount].hash = path;
	manager->textures[manager->textureCount].asset = mvCreateCubeTexture(path);
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
