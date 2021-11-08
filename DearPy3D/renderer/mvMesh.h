#pragma once

#include <memory>
#include "mvMath.h"
#include "mvBuffer.h"
#include "mvPipeline.h"
#include "mvMaterials.h"
#include "mvTypes.h"

struct mvObjMesh;
struct mvAssetManager;

struct mvMesh
{
    std::string name;
    mvAssetID   indexBuffer = -1;
    mvAssetID   vertexBuffer = -1;
    mvAssetID   diffuseTexture = -1;
    mvAssetID   normalTexture = -1;
    mvAssetID   specularTexture = -1;
    mvAssetID   albedoTexture = -1;
    mvAssetID   metalRoughnessTexture = -1;
    mvAssetID   phongMaterialID = -1;
    mvAssetID   pbrMaterialID = -1;
    b8          pbr = false;
};

mvMesh mvCreateTexturedCube(mvAssetManager& assetManager, float sideLength = 1.0f);
mvMesh mvCreateTexturedQuad(mvAssetManager& assetManager, float sideLength = 1.0f);
void mvLoadOBJAssets(mvAssetManager& assetManager, const std::string& root, const std::string& file);
