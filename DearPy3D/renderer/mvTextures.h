#pragma once

#include "mvDearPy3D.h"
#include <string>
#include <vulkan/vulkan.h>
#include <imgui.h>

namespace DearPy3D
{
	mvTexture create_texture     (mvGraphics& graphics, const char* file);
	mvTexture create_texture_cube(mvGraphics& graphics, const char* path);
	mvTexture create_texture     (mvGraphics& graphics, u32 width, u32 height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect);
	mvTexture create_texture_cube(mvGraphics& graphics, u32 width, u32 height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect);

	mvTextureManager create_texture_manager();
	void             cleanup_texture_manager(mvGraphics& graphics, mvTextureManager& manager);
	mvAssetID        register_texture(mvTextureManager& manager, const std::string& tag, mvTexture texture);
	mvAssetID        register_texture_safe_load(mvGraphics& graphics, mvTextureManager& manager, const std::string& tag);
	mvAssetID        register_texture_cube_safe_load(mvGraphics& graphics, mvTextureManager& manager, const std::string& tag);

	void transition_image_layout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange,
		VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	void transition_image_layout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout,
		VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
}

struct mvTexture
{
	VkImage               textureImage;
	VkDeviceMemory        textureImageMemory;
	std::string           file;
	VkDescriptorImageInfo imageInfo;
	ImTextureID           imguiID;
	unsigned              mipLevels;
};

struct mvTextureManager
{
	std::string* textureKeys;
	unsigned     maxTextureCount;
	unsigned     textureCount;
	mvTexture*   textures;
};