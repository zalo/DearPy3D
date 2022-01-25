#pragma once

#include "mvMath.h"

// forward declarations
struct mvViewport;
struct mvCamera;
struct mvOrthoCamera;

// updates
void update_fps_camera(mvViewport& viewport, mvCamera& camera, f32 dt, f32 travelSpeed, f32 rotationSpeed);

struct mvCamera
{
    mvVec3 pos         = { 0.0f, 0.0f, 0.0f };
    mvVec3 front       = { 0.0f, 0.0f, 1.0f };
    mvVec3 up          = { 0.0f, 1.0f, 0.0f };
    f32    pitch       = 0.0f;
    f32    yaw         = 0.0f;
    f32    aspect      = 1.0f;
    f32    fieldOfView = MV_PI_4;
    f32    nearZ       = 0.1f;
    f32    farZ        = 400.0f;
};

struct mvOrthoCamera
{
    mvVec3 pos    = { 0.0f, 100.0f, 0.0f };
    mvVec3 dir    = { 0.0f, -1.0f, 0.0f };
    mvVec3 up     = { 1.0f, 0.0f, 0.0f };
    f32    left   = -100.0f;
    f32    right  =  100.0f;
    f32    bottom = -100.0f;
    f32    top    =  100.0f;
    f32    nearZ  = -101.0f;
    f32    farZ   =  101.0f;
};
