#pragma once

#include <string>
#include <vector>
#include "mvTypes.h"
#include "mvTextures.h"
#include "mvMaterials.h"
#include "mvBuffer.h"
#include "mvMesh.h"
#include "mvPipeline.h"

struct mvMeshAsset;
struct mvBufferAsset;
struct mvTextureAsset;
struct mvPhongMaterialAsset;
struct mvSamplerAsset;
struct mvPipelineAsset;

struct mvAssetManager
{
	// samplers
	u32                   maxSamplerCount = 50u;
	u32                   samplerCount = 0u;
	mvSamplerAsset*       samplers = nullptr;

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
};

void mvInitializeAssetManager   (mvAssetManager* manager);
void mvPrepareResizeAssetManager(mvAssetManager* manager);
void mvCleanupAssetManager      (mvAssetManager* manager);

mvAssetID mvGetTextureAsset      (mvAssetManager* manager, const std::string& path);
mvAssetID mvGetBufferAsset       (mvAssetManager* manager, void* data, u64 count, u64 size, VkBufferUsageFlags flags, const std::string& tag);
mvAssetID mvGetDynamicBufferAsset(mvAssetManager* manager, void* data, u64 count, u64 size, VkBufferUsageFlags flags, const std::string& tag);
mvAssetID mvGetPhongMaterialAsset(mvAssetManager* manager, mvMaterialData materialData, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, const char* vertexShader, const char* pixelShader);
mvAssetID mvGetSamplerAsset      (mvAssetManager* manager);
mvAssetID mvGetPipelineAsset     (mvAssetManager* manager, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, const char* vertexShader, const char* pixelShader);

mvAssetID mvRegistryMeshAsset    (mvAssetManager* manager, mvMesh mesh);

struct mvSamplerAsset
{
	std::string hash;
	mvSampler   sampler;
};

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
