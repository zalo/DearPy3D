#pragma once

#include "mvMesh.h"
#include "mvMaterials.h"
#include "mvPipeline.h"
#include "mvDescriptors.h"
#include "mvTextures.h"

// forward declarations
struct mvModel;
struct mvPassSpecification;
struct mvPass;
struct mvSkybox;
struct mvRendererContext;

struct mvPassSpecification
{
	std::string name;
	mvAssetID   pipeline           = MV_INVALID_ASSET_ID;
	f32         clearColorValue[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	f32         clearDepthValue    = 1.0f;
	f32         depthBias          = 0.0f;
	f32         slopeDepthBias     = 0.0f;
	f32         width              = 0.0f;
	f32         height             = 0.0f;
	VkFormat    colorFormat        = VK_FORMAT_R8G8B8A8_UNORM;
	VkFormat    depthFormat        = VK_FORMAT_D32_SFLOAT;
	b8          hasColor           = true;
	b8          hasDepth           = true;
	b8          mainPass           = false;
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
	mvMesh              meshes[256]; // lights, cameras, etc.
	u32                 meshCount = 0u;
};

namespace Renderer
{

	mvRendererContext create_renderer_context (mvGraphics& graphics);
	void              cleanup_renderer_context(mvRendererContext& ctx);

	void begin_frame(mvRendererContext& ctx);
	void end_frame  (mvRendererContext& ctx);
	void begin_pass (mvRendererContext& ctx, VkCommandBuffer commandBuffer, mvPass& pass);
	void end_pass   (VkCommandBuffer commandBuffer);

	// pass creation/destruction
	mvPass create_omni_shadow_pass(mvGraphics& graphics, mvPassSpecification specification);
	mvPass create_offscreen_pass  (mvGraphics& graphics, mvPassSpecification specification);
	mvPass create_depth_pass      (mvGraphics& graphics, mvPassSpecification specification);
	void   cleanup_pass           (mvGraphics& graphics, mvPass& pass);

	void render_mesh(mvGraphics& graphics, mvDescriptorSet& decriptorSet, VkPipelineLayout pipelineLayout, mvMesh& mesh, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection);
	void render_scene(mvGraphics& graphics, mvModel& model, mvMaterialManager& mManager, VkPipelineLayout pipelineLayout, mvScene& scene, mvMat4 cam, mvMat4 proj);
	void render_skybox(mvSkybox& skybox, mvGraphics& graphics, VkPipelineLayout pipelineLayout, mvMat4 cam, mvMat4 proj);

	void render_mesh_shadow(mvGraphics& graphics, VkPipelineLayout pipelineLayout, mvMesh& mesh, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection);
	void render_mesh_omni_shadow(mvGraphics& graphics, VkPipelineLayout pipelineLayout, mvMesh& mesh, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection, mvVec4 lightPos);
	void render_scene_shadows(mvGraphics& graphics, mvModel& model, VkPipelineLayout pipelineLayout, mvScene& scene, mvMat4 cam, mvMat4 proj);
	void render_scene_omni_shadows(mvGraphics& graphics, mvModel& model, VkPipelineLayout pipelineLayout, mvScene& scene, mvMat4 cam, mvMat4 proj, mvVec4 lightPos);

	void update_descriptors(mvGraphics& graphics, mvModel& model, mvRendererContext& rctx);
}