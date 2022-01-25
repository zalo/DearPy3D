#pragma once

// in the process of removing this

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
struct mvSceneAsset;
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

	// nodes	       	  
	u32                   maxNodeCount = 500u;
	u32                   nodeCount = 0u;
	mvNodeAsset*          nodes = nullptr;
};

// startup/shutdown/resize
void mvInitializeAssetManager   (mvAssetManager* manager);
void mvCleanupAssetManager      (mvGraphics& graphics, mvAssetManager* manager);
void mvShowAssetManager         (mvAssetManager& manager);

// registering
mvAssetID mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvScene asset);
mvAssetID mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvMesh asset);
mvAssetID mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvBuffer asset);
mvAssetID mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvNode asset);
mvAssetID mvRegisterAsset(mvAssetManager* manager, const std::string& tag, mvTexture asset);

// ID retrieval
mvAssetID mvGetTextureAssetID (mvGraphics& graphics, mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetTextureAssetID2(mvGraphics& graphics, mvAssetManager* manager, const std::string& tag);
mvAssetID mvGetBufferAssetID  (mvAssetManager* manager, const std::string& tag);

// asset retrieval
mvMesh* mvGetRawMeshAsset(mvAssetManager* manager, const std::string& tag);

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