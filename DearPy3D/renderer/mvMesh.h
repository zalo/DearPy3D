#pragma once

#include "mvDearPy3D.h"
#include "mvBuffer.h"
#include "mvPipeline.h"
#include "mvMaterials.h"

struct mvMesh
{
    std::string name;
    mvBuffer    indexBuffer;
    mvBuffer    vertexBuffer;
    mvAssetID   diffuseTexture;
    mvAssetID   normalTexture;
    mvAssetID   specularTexture;
    mvAssetID   albedoTexture;
    mvAssetID   metalRoughnessTexture;
    mvAssetID   phongMaterialID;
};
