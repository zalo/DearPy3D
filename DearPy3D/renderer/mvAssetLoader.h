#pragma once

#include "mvScene.h"

// forward declarations
struct mvRendererContext;

mvModel load_obj_model(mvRendererContext& rctx, const std::string& root, const std::string& file);