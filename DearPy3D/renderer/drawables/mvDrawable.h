#pragma once

#include <memory>
#include "mvMath.h"
#include "mvVertexBuffer.h"
#include "mvIndexBuffer.h"
#include "mvPipeline.h"
#include "mvDeletionQueue.h"
#include "mvMaterial.h"

namespace DearPy3D {

    struct mvDrawable
    {
        std::shared_ptr<mvIndexBuffer>  indexBuffer;
        std::shared_ptr<mvVertexBuffer> vertexBuffer;
        glm::vec3 pos;
        glm::vec3 rot;
    };

    void mvCleanupDrawable(mvDrawable& drawable);
        
}
