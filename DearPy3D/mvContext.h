#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "mvViewport.h"
#include "mvGraphicsContext.h"

namespace DearPy3D {

	//-----------------------------------------------------------------------------
    // forward declarations
    //-----------------------------------------------------------------------------
	struct mvContext;
    struct mvIO;


    //-----------------------------------------------------------------------------
    // public API
    //-----------------------------------------------------------------------------
	extern mvContext* GContext;

    struct mvIO
    {

    };

    struct mvContext
    {
        mvIO              IO;
        mvViewport        viewport;
        mvGraphicsContext graphics;
    };

    void CreateContext();
    void DestroyContext();
}