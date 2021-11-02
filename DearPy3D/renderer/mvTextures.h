#pragma once

#include <string>
#include <vulkan/vulkan.h>

struct mvTexture
{
	VkImage        textureImage       = VK_NULL_HANDLE;
	VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
	VkImageView    textureImageView   = VK_NULL_HANDLE;
};

struct mvSampler
{
	VkSampler textureSampler;
};

mvSampler mvCreateSampler();
mvTexture mvCreateTexture(const std::string& file);
