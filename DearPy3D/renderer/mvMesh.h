#pragma once

#include "mvMath.h"
#include "mvBuffer.h"
#include "mvPipeline.h"
#include "mvMaterials.h"
#include "mvTypes.h"

// forward declarations
struct mvObjMesh;
struct mvAssetManager;
struct mvGraphics;
struct mvMesh;
struct mvRendererContext;

mvMesh create_textured_cube(mvGraphics& graphics, mvRendererContext& rctx, float sideLength = 1.0f);
mvMesh create_textured_quad(mvGraphics& graphics, mvRendererContext& rctx, float sideLength = 1.0f);

struct mvMesh
{
    std::string name;
    mvBuffer    indexBuffer;
    mvBuffer    vertexBuffer;
    mvAssetID   diffuseTexture = -1;
    mvAssetID   normalTexture = -1;
    mvAssetID   specularTexture = -1;
    mvAssetID   albedoTexture = -1;
    mvAssetID   metalRoughnessTexture = -1;
    mvAssetID   phongMaterialID = -1;
};
