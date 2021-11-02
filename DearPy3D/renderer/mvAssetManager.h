#pragma once

#include <string>
#include "mvTypes.h"
#include "mvTextures.h"
#include "mvMaterials.h"
#include "mvBuffer.h"
#include "mvMesh.h"

struct mvMeshAsset;
struct mvBufferAsset;
struct mvTextureAsset;

struct mvAssetManager
{

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
};

void mvInitializeAssetManager   (mvAssetManager* manager);
void mvPrepareResizeAssetManager(mvAssetManager* manager);
void mvCleanupAssetManager      (mvAssetManager* manager);

mvAssetID mvGetTextureAsset      (mvAssetManager* manager, const std::string& path);
mvAssetID mvGetBufferAsset       (mvAssetManager* manager, void* data, u64 count, u64 size, VkBufferUsageFlags flags, const std::string& tag);
mvAssetID mvGetDynamicBufferAsset(mvAssetManager* manager, void* data, u64 count, u64 size, VkBufferUsageFlags flags, const std::string& tag);
mvAssetID mvRegistryMeshAsset    (mvAssetManager* manager, mvMesh mesh);

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
