#pragma once

#include <memory>
#include "mvMath.h"
#include "mvBuffer.h"
#include "mvPipeline.h"
#include "mvDeletionQueue.h"
#include "mvMaterial.h"

struct mvObjMesh;

struct mvMesh
{
    mvBuffer  indexBuffer;
    mvBuffer  vertexBuffer;
    mvVec3 pos;
    mvVec3 rot;
};

mvMesh mvCreateTexturedCube(float sideLength = 1.0f);
mvMesh mvCreateTexturedQuad(float sideLength = 1.0f);
mvMesh mvCreateObjMesh     (mvObjMesh& mesh);
void   mvCleanupMesh       (mvMesh& mesh);
