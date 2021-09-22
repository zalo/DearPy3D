#include "mvDrawable.h"
#include "mvContext.h"

namespace DearPy3D {


    void mvCleanupDrawable(mvDrawable& drawable)
    {
        mvCleanupBuffer(drawable.indexBuffer);
        drawable.indexBuffer.buffer = VK_NULL_HANDLE;
        drawable.indexBuffer.memoryAllocation = VK_NULL_HANDLE;
        drawable.indexBuffer.count = 0u;

        mvCleanupBuffer(drawable.vertexBuffer);
        drawable.vertexBuffer.buffer = VK_NULL_HANDLE;
        drawable.vertexBuffer.memoryAllocation = VK_NULL_HANDLE;
        drawable.vertexBuffer.count = 0u;
    }
    
}
