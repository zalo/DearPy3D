#pragma once

#include <string>
#include <vulkan/vulkan.h>
#include <imgui.h>
#include "mvTypes.h"

struct mvTexture
{
	VkImage               textureImage       = VK_NULL_HANDLE;
	VkDeviceMemory        textureImageMemory = VK_NULL_HANDLE;
	std::string           file;
	VkDescriptorImageInfo imageInfo;
	ImTextureID           imguiID;
	u32                   mipLevels;
};

mvTexture mvCreateTexture(const std::string& file);
mvTexture mvCreateCubeTexture(const std::string& file);
mvTexture mvCreateTexture(u32 width, u32 height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect);
