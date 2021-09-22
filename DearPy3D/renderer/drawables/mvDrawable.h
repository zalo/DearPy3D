#pragma once

#include <memory>
#include "mvMath.h"
#include "mvBuffer.h"
#include "mvPipeline.h"
#include "mvDeletionQueue.h"
#include "mvMaterial.h"

namespace DearPy3D {

    struct mvDrawable
    {
        mvBuffer  indexBuffer;
        mvBuffer  vertexBuffer;
        glm::vec3 pos;
        glm::vec3 rot;
    };

    void mvCleanupDrawable(mvDrawable& drawable);
        
}
