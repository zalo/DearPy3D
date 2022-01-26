#pragma once

#include "mvTypes.h"
#include "mvDescriptors.h"
#include "mvMesh.h"

// forward declarations
struct mvAssetManager;
struct mvGraphics;
struct mvRendererContext;

struct mvSkybox
{
    mvDescriptorSet descriptorSet;
    mvMesh          mesh;
};

mvSkybox create_skybox            (mvRendererContext& rctx);
void     update_skybox_descriptors(mvRendererContext& rctx, mvSkybox& skybox, mvAssetID texture);
void     bind_skybox              (mvGraphics& graphics, mvSkybox& skybox);