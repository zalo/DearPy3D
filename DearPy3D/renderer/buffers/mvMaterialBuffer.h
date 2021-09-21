#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include "mvMath.h"

namespace DearPy3D {

	struct mvMaterialData
	{
		glm::vec4 materialColor = { 0.45f, 0.45f, 0.85f, 1.0f };
		//-------------------------- ( 16 bytes )

		glm::vec3 specularColor = { 0.18f, 0.18f, 0.18f };
		float specularGloss = 8.0f;
		//-------------------------- ( 16 bytes )

		float normalMapWeight = 1.0f;
		int useTextureMap = false;
		int useNormalMap = false;
		int useSpecularMap = false;
		//-------------------------- ( 16 bytes )

		int useGlossAlpha = false;
		int hasAlpha = false;
		char _pad1[8];
		//-------------------------- ( 16 bytes )
		// 
		//-------------------------- ( 4 * 16 = 64 bytes )
	};

	struct mvMaterialBuffer
	{
		std::vector<VkBuffer>       buffer;
		std::vector<VkDeviceMemory> bufferMemory;
	};

	mvMaterialBuffer mvCreateMaterialBuffer();
	void             mvBind                 (mvMaterialBuffer& material, mvMaterialData data, uint32_t index);
	void             mvCleanupMaterialBuffer(mvMaterialBuffer& material);

}