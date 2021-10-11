#pragma once

#include <string>
#include <vulkan/vulkan.h>

struct mvSampler
{
	VkSampler textureSampler;
};

mvSampler mvCreateSampler();
void      mvCleanupSampler(mvSampler& sampler);
