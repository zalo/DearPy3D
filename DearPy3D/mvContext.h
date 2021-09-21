#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "mvViewport.h"
#include "mvGraphics.h"

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
        mvIO       IO;
        mvViewport viewport;
        mvGraphics graphics;
    };

    void CreateContext();
    void DestroyContext();
}