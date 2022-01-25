#pragma once

#include "mvMesh.h"

// forward declarations
struct mvAssetManager;
struct mvPassSpecification;
struct mvPass;
struct mvSkybox;

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

namespace Renderer
{
	void begin_frame(mvGraphics& graphics);
	void end_frame  (mvGraphics& graphics);
	void begin_pass (mvGraphics& graphics, mvPipelineManager& pmManager, VkCommandBuffer commandBuffer, mvPass& pass);
	void end_pass   (VkCommandBuffer commandBuffer);

	mvPass create_omni_shadow_pass(mvGraphics& graphics, mvAssetManager& am, mvPassSpecification specification);
	mvPass create_primary_pass(mvGraphics& graphics, mvAssetManager& am, mvPassSpecification specification);
	mvPass create_offscreen_pass(mvGraphics& graphics, mvAssetManager& am, mvPassSpecification specification);
	mvPass create_depth_pass(mvGraphics& graphics, mvAssetManager& am, mvPassSpecification specification);

	void cleanup_pass(mvGraphics& graphics, mvPass& pass);

	void render_mesh(mvGraphics& graphics, mvDescriptorSet& decriptorSet, VkPipelineLayout pipelineLayout, mvMesh& mesh, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection);
	void render_scene(mvGraphics& graphics, mvAssetManager& am, mvMaterialManager& mManager, VkPipelineLayout pipelineLayout, mvScene& scene, mvMat4 cam, mvMat4 proj);
	void render_skybox(mvSkybox& skybox, mvGraphics& graphics, VkPipelineLayout pipelineLayout, mvMat4 cam, mvMat4 proj);

	void render_mesh_shadow(mvGraphics& graphics, VkPipelineLayout pipelineLayout, mvMesh& mesh, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection);
	void render_mesh_omni_shadow(mvGraphics& graphics, VkPipelineLayout pipelineLayout, mvMesh& mesh, mvMat4 accumulatedTransform, mvMat4 camera, mvMat4 projection, mvVec4 lightPos);
	void render_scene_shadows(mvGraphics& graphics, mvAssetManager& am, VkPipelineLayout pipelineLayout, mvScene& scene, mvMat4 cam, mvMat4 proj);
	void render_scene_omni_shadows(mvGraphics& graphics, mvAssetManager& am, VkPipelineLayout pipelineLayout, mvScene& scene, mvMat4 cam, mvMat4 proj, mvVec4 lightPos);

	void update_descriptors(mvGraphics& graphics, mvAssetManager& am, mvMaterialManager& mManager);
}