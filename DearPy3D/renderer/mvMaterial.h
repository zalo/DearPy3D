#pragma once

#include <memory>
#include "mvPipeline.h"
#include "mvDeletionQueue.h"
#include "mvMaterialBuffer.h"
#include "mvTexture.h"
#include "mvSampler.h"

namespace DearPy3D {

	struct mvMaterial
	{
		mvPipeline        pipeline;
		mvTexture         texture;
		mvSampler         sampler;
		uint32_t          offsetIndex = 0u;
		mvMaterialBuffer  materialBuffer;
	};

	mvMaterial mvCreateMaterial();
	void       mvCleanupMaterial(mvMaterial& material);
	void       mvBind           (mvMaterial& material, uint32_t index, mvMaterialData data);

}