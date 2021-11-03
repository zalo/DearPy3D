#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "mvViewport.h"
#include "mvGraphics.h"

#define MV_MAX_FRAMES_IN_FLIGHT 2

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
    std::string shaderDirectory;
};

struct mvContext
{
    mvIO       IO;
    mvViewport viewport;
    mvGraphics graphics;
};

void CreateContext();
void DestroyContext();

#define mv_internal static
#define mv_local_persist static
#define mv_global static