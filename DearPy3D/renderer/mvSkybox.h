#pragma once

#include "mvTypes.h"
#include "mvDescriptors.h"
#include "mvMesh.h"

// forward declarations
struct mvAssetManager;
struct mvGraphics;

struct mvSkybox
{
    mvDescriptorSet descriptorSet;
    mvMesh          mesh;
};

mvSkybox create_skybox            (mvGraphics& graphics, mvAssetManager& am, mvMaterialManager& mManager, mvDescriptorManager& dsManager, mvPipelineManager& pmManager);
void     update_skybox_descriptors(mvGraphics& graphics, mvAssetManager& am, mvSkybox& skybox, mvAssetID texture);
void     bind_skybox              (mvGraphics& graphics, mvSkybox& skybox);