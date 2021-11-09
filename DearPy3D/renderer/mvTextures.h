#pragma once

#include <string>
#include <vulkan/vulkan.h>
#include <imgui.h>

struct mvTexture
{
	VkImage               textureImage       = VK_NULL_HANDLE;
	VkDeviceMemory        textureImageMemory = VK_NULL_HANDLE;
	std::string           file;
	VkDescriptorImageInfo imageInfo;
	ImTextureID           imguiID;
};

mvTexture mvCreateTexture(const std::string& file);
