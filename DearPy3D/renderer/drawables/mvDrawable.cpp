#include "mvDrawable.h"
#include "mvContext.h"

namespace DearPy3D {


    void mvCleanupDrawable(mvDrawable& drawable)
    {
        drawable.indexBuffer->cleanup();
        drawable.vertexBuffer->cleanup();
        drawable.indexBuffer = nullptr;
        drawable.vertexBuffer = nullptr;
    }
    
}
