#pragma once

#include <string>
#include <vulkan/vulkan.h>
#include <imgui.h>
#include "mvTypes.h"

// forware declarations
struct mvGraphics;
struct mvTexture;
struct mvTextureManager;

mvTexture create_texture(mvGraphics& graphics, const char* file);
mvTexture create_texture_cube(mvGraphics& graphics, const char* path);
mvTexture create_texture(mvGraphics& graphics, u32 width, u32 height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect);
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


struct mvTexture
{
	VkImage               textureImage       = VK_NULL_HANDLE;
	VkDeviceMemory        textureImageMemory = VK_NULL_HANDLE;
	std::string           file;
	VkDescriptorImageInfo imageInfo;
	ImTextureID           imguiID;
	u32                   mipLevels;
};

struct mvTextureManager
{
	std::string* textureKeys = nullptr;
	u32          maxTextureCount = 500u;
	u32          textureCount = 0u;
	mvTexture*   textures = nullptr;
};