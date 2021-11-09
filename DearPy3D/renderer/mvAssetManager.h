#pragma once

#include <string>
#include <vector>
#include "mvTypes.h"
#include "mvTextures.h"
#include "mvMaterials.h"
#include "mvBuffer.h"
#include "mvMesh.h"
#include "mvPipeline.h"
#include "mvScene.h"

struct mvMeshAsset;
struct mvBufferAsset;
struct mvTextureAsset;
struct mvPhongMaterialAsset;
struct mvPipelineAsset;
struct mvPipelineLayoutAsset;
struct mvSceneAsset;
struct mvDescriptorSetLayoutAsset;
struct mvNodeAsset;
struct mvSceneAsset;

struct mvAssetManager
{
	// scenes
	u32                   maxSceneCount = 50u;
	u32                   sceneCount = 0u;
	mvSceneAsset*         scenes = nullptr;

	// textures
	u32                   maxTextureCount = 500u;
	u32                   textureCount = 0u;
	mvTextureAsset*       textures = nullptr;
				       
	// buffers	       
	u32                   maxBufferCount = 500u;
	u32                   bufferCount = 0u;
	mvBufferAsset*        buffers = nullptr;

	// dynamic buffers	       
	u32                   maxDynBufferCount = 500u;
	u32                   dynBufferCount = 0u;
	mvBufferAsset*        dynBuffers = nullptr;
				       	  
	// meshes	       	  
	u32                   maxMeshCount = 500u;
	u32                   meshCount = 0u;
	mvMeshAsset*          meshes = nullptr;

	// phong materials	       	  
	u32                   maxPhongMaterialCount = 500u;
	u32                   phongMaterialCount = 0u;
	mvPhongMaterialAsset* phongMaterials = nullptr;

	// pipelines	       	  
	u32                   maxPipelineCount = 50u;
	u32                   pipelineCount = 0u;
	mvPipelineAsset*      pipelines = nullptr;

	// pipeline layouts       	  
	u32                    maxPipelineLayoutCount = 50u;
	u32                    pipelineLayoutCount = 0u;
	mvPipelineLayoutAsset* pipelineLayouts = nullptr;

	// descriptor set layouts       	  
	u32                         maxDescriptorSetLayoutCount = 50u;
	u32                         descriptorSetLayoutCount = 0u;
	mvDescriptorSetLayoutAsset* descriptorSetLayouts = nullptr;

	// nodes	       	  
	u32                   maxNodeCount = 500u;
	u32                   nodeCount = 0u;
	mvNodeAsset*          nodes = nullptr;
};

void mvInitializeAssetManager   (mvAssetManager* manager);
void mvPrepareResizeAssetManager(mvAssetManager* manager);
void mvCleanupAssetManager      (mvAssetManager* manager);

mvAssetID mvGetTextureAsset       (mvAssetManager* manager, const std::string& path);
mvAssetID mvGetBufferAsset        (mvAssetManager* manager, void* data, u64 count, u64 size, VkBufferUsageFlags flags, const std::string& tag);
mvAssetID mvGetDynamicBufferAsset (mvAssetManager* manager, void* data, u64 count, u64 size, VkBufferUsageFlags flags, const std::string& tag);
mvAssetID mvGetPhongMaterialAsset (mvAssetManager* manager, mvMaterialData materialData, const char* vertexShader, const char* pixelShader);
mvAssetID mvGetPipelineAsset      (mvAssetManager* manager, mvPipelineSpec& spec);
mvAssetID mvGetPipelineLayoutAsset(mvAssetManager* manager, std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
mvAssetID mvGetPipelineLayoutAsset(mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetSceneAsset         (mvAssetManager* manager, mvSceneData sceneData);

mvAssetID mvRegisterDescriptorSetLayoutAsset(mvAssetManager* manager, VkDescriptorSetLayout layout, const std::string& tag);
mvAssetID mvGetDescriptorSetLayoutAsset(mvAssetManager* manager, const std::string& tag);

mvAssetID mvRegistryMeshAsset     (mvAssetManager* manager, mvMesh mesh);
mvAssetID mvRegistryNodeAsset     (mvAssetManager* manager, mvNode node);
mvAssetID mvRegistrySceneAsset    (mvAssetManager* manager, mvScene scene);

std::string mvCreateHash(mvMaterialData materialData);

void mvShowAssetManager(mvAssetManager& assetManager);

struct mvMeshAsset
{
	mvMesh mesh;
};

struct mvBufferAsset
{
	std::string hash;
	mvBuffer    buffer;
};

struct mvTextureAsset
{
	std::string hash;
	mvTexture   texture;
};

struct mvPhongMaterialAsset
{
	std::string hash;
	mvMaterial  material;
};

struct mvPipelineAsset
{
	std::string hash;
	mvPipeline  pipeline;
};

struct mvPipelineLayoutAsset
{
	std::string hash;
	mvPipelineLayout layout;
};

struct mvDescriptorSetLayoutAsset
{
	std::string           hash;
	VkDescriptorSetLayout layout;
};

struct mvSceneAsset
{
	std::string hash;
	mvScene     scene;
};

struct mvNodeAsset
{
	mvNode node;
};