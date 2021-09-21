#include "mvDrawable.h"
#include "mvContext.h"

namespace DearPy3D {


    void mvCleanupDrawable(mvDrawable& drawable)
    {
        mvCleanupIndexBuffer(drawable.indexBuffer);
        drawable.indexBuffer.buffer = VK_NULL_HANDLE;
        drawable.indexBuffer.memoryAllocation = VK_NULL_HANDLE;
        drawable.indexBuffer.indices.clear();

        mvCleanupVertexBuffer(drawable.vertexBuffer);
        drawable.vertexBuffer.buffer = VK_NULL_HANDLE;
        drawable.vertexBuffer.memoryAllocation = VK_NULL_HANDLE;
        drawable.vertexBuffer.vertices.clear();
    }
    
}
