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
    mvAssetID   indexBuffer;
    mvAssetID   vertexBuffer;
    mvVec3      pos;
    mvVec3      rot;
};

mvMesh mvCreateTexturedCube(mvAssetManager& assetManager, float sideLength = 1.0f);
mvMesh mvCreateTexturedQuad(mvAssetManager& assetManager, float sideLength = 1.0f);
mvMesh mvCreateObjMesh     (mvObjMesh& mesh);
