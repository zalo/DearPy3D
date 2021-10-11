#pragma once

#include <vulkan/vulkan.h>
#include <vector>

enum mvVertexElementType
{
	Position2D,
	Position3D,
	Texture2D,
	Normal,
	Tangent,
	Bitangent,
	Color
};

struct mvVertexLayout
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	std::vector<VkVertexInputBindingDescription>   bindingDescriptions;
};

mvVertexLayout mvCreateVertexLayout(std::vector<mvVertexElementType> elements);
