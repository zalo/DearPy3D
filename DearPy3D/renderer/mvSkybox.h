#pragma once

#include "mvTypes.h"
#include "mvDescriptors.h"

struct mvAssetManager;

struct mvSkybox
{
    mvDescriptorSet descriptorSet;
};

mvSkybox mvCreateSkybox          (mvAssetManager& am);
void    mvUpdateSkyboxDescriptors(mvAssetManager& am, mvSkybox& skybox, mvAssetID texture);
void    mvBindSkybox             (mvAssetManager& am, mvSkybox& skybox);