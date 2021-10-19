#pragma once

#include "mvMath.h"

static constexpr float CameraTravelSpeed = 12.0f;
static constexpr float CameraRotationSpeed = 0.004f;

struct mvCamera
{
    mvVec3 pos;
    float pitch = 0.0f;
    float yaw = 0.0f;
    float aspect = 1.0f;
};

mvMat4 mvBuildCameraMatrix    (mvCamera& camera);
mvMat4 mvBuildProjectionMatrix(mvCamera& camera);
void   mvRotateCamera         (mvCamera& camera, float dx, float dy);
void   mvTranslateCamera      (mvCamera& camera, float dx, float dy, float dz);
