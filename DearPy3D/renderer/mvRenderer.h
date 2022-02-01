#pragma once

#include "mvDearPy3D.h"
#include "mvMesh.h"
#include "mvMaterials.h"
#include "mvPipeline.h"
#include "mvDescriptors.h"
#include "mvTextures.h"

struct mvPassSpecification
{
	std::string name;
	mvAssetID   pipeline;
	sVec4       clearColorValue;
	float       clearDepthValue;
	float       depthBias;
	float       slopeDepthBias;
	float       width;
	float       height;
	VkFormat    colorFormat;
	VkFormat    depthFormat;
	bool        hasColor;
	bool        hasDepth;
	bool        mainPass;
};

struct mvPass
{
	mvPassSpecification        specification;
	VkRenderPass               renderPass;
	VkExtent2D                 extent;
	std::vector<VkFramebuffer> frameBuffers;
	std::vector<mvTexture>     colorTextures;
	std::vector<mvTexture>     depthTextures;
	VkViewport                 viewport;
	mvPipelineSpec             pipelineSpec;
};

struct mvRendererContext
{
	mvGraphics*         graphics; // store for convencience
	mvPipelineManager   pipelineManager;
	mvDescriptorManager descriptorManager;
	mvMaterialManager   materialManager;
	mvTextureManager    textureManager;
	mvMesh              meshes[16]; // lights, cameras, etc.
	unsigned            meshCount;
};

namespace DearPy3D
{

	mvRendererContext create_renderer_context (mvGraphics& graphics);
	void              cleanup_renderer_context(mvRendererContext& ctx);
	void              begin_frame(mvRendererContext& ctx);
	void              end_frame  (mvRendererContext& ctx);
	void              begin_pass (mvRendererContext& ctx, VkCommandBuffer commandBuffer, mvPass& pass);
	void              end_pass   (VkCommandBuffer commandBuffer);
	void              cleanup_pass(mvGraphics& graphics, mvPass& pass);
}