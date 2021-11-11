#pragma once

#include <string>
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

// startup/shutdown/resize
void mvInitializeAssetManager   (mvAssetManager* manager);
void mvPrepareResizeAssetManager(mvAssetManager* manager);
void mvCleanupAssetManager      (mvAssetManager* manager);
void mvShowAssetManager         (mvAssetManager& manager);

// registering
mvAssetID mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvPipeline asset);
mvAssetID mvRegisterAsset(mvAssetManager* manager, const std::string& tag, VkPipelineLayout asset);
mvAssetID mvRegisterAsset(mvAssetManager* manager, const std::string& tag, VkDescriptorSetLayout asset);
mvAssetID mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvScene asset);
mvAssetID mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvMesh asset);
mvAssetID mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvBuffer asset);
mvAssetID mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvMaterial asset);
mvAssetID mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvNode asset);

// ID retrieval
mvAssetID mvGetPipelineAssetID           (mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetPipelineLayoutAssetID     (mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetDescriptorSetLayoutAssetID(mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetSceneAssetID              (mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetTextureAssetID            (mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetBufferAssetID             (mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetMaterialAssetID           (mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetNodeAssetID               (mvAssetManager* manager, const std::string& tag);

// asset retrieval
mvPipeline*           mvGetRawPipelineAsset           (mvAssetManager* manager, const std::string& tag);
VkPipelineLayout      mvGetRawPipelineLayoutAsset     (mvAssetManager* manager, const std::string& tag);
VkDescriptorSetLayout mvGetRawDescriptorSetLayoutAsset(mvAssetManager* manager, const std::string& tag);
mvScene*              mvGetRawSceneAsset              (mvAssetManager* manager, const std::string& tag);
mvTexture*            mvGetRawTextureAsset            (mvAssetManager* manager, const std::string& tag);
mvMesh*               mvGetRawMeshAsset               (mvAssetManager* manager, const std::string& tag);
mvBuffer*             mvGetRawBufferAsset             (mvAssetManager* manager, const std::string& tag);
mvMaterial*           mvGetRawMaterialAsset           (mvAssetManager* manager, const std::string& tag);
mvNode*               mvGetRawNodeAsset               (mvAssetManager* manager, const std::string& tag);

struct mvMeshAsset
{
	std::string hash;
	mvMesh      asset;
};

struct mvBufferAsset
{
	std::string hash;
	mvBuffer    asset;
};

struct mvTextureAsset
{
	std::string hash;
	mvTexture   asset;
};

struct mvPhongMaterialAsset
{
	std::string hash;
	mvMaterial  asset;
};

struct mvPipelineAsset
{
	std::string hash;
	mvPipeline  asset;
};

struct mvPipelineLayoutAsset
{
	std::string      hash;
	VkPipelineLayout asset;
};

struct mvDescriptorSetLayoutAsset
{
	std::string           hash;
	VkDescriptorSetLayout asset;
};

struct mvSceneAsset
{
	std::string hash;
	mvScene     asset;
};

struct mvNodeAsset
{
	std::string hash;
	mvNode      asset;
};