#include "mvCamera.h"
#include <imgui.h>
#include <cmath>
#include "mvViewport.h"

template<typename T>
mv_local_persist T
wrap_angle(T theta) noexcept
{
    constexpr T twoPi = (T)2 * (T)MV_PI;
    const T mod = (T)fmod(theta, twoPi);
    if (mod > (T)MV_PI)
        return mod - twoPi;
    else if (mod < -(T)MV_PI)
        return mod + twoPi;
    return mod;
}

mv_internal
f32 mvClamp(f32 value, f32 minValue, f32 maxValue)
{
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

mvMat4 
create_fps_view(mvCamera& camera)
{
    return mvFPSViewRH(camera.pos, camera.pitch, camera.yaw);
}

mvMat4
create_ortho_view(mvOrthoCamera& camera)
{
    return mvLookAtRH(camera.pos, camera.pos - camera.dir, camera.up);
}

mvMat4
create_ortho_projection(mvOrthoCamera& camera)
{
    return mvOrthoRH(
        camera.left, 
        camera.right, 
        camera.bottom, 
        camera.top, 
        camera.nearZ, 
        camera.farZ);
}

mvMat4
create_lookat_view(mvCamera& camera)
{
    mvVec3 direction{};
    direction.x = cos((camera.yaw)) * cos((camera.pitch));
    direction.y = sin((camera.pitch));
    direction.z = sin((camera.yaw)) * cos((camera.pitch));
    direction = mvNormalize(direction);
    return mvLookAtRH(camera.pos, camera.pos + direction, camera.up);
}

mvMat4
create_perspective_projection(mvCamera& camera)
{
    return mvPerspectiveRH(camera.fieldOfView, camera.aspect, camera.nearZ, camera.farZ);
}

void
update_fps_camera(mvViewport& viewport, mvCamera& camera, f32 dt, f32 travelSpeed, f32 rotationSpeed)
{
    // for now, we will just use imgui's input
    if (ImGui::GetIO().KeysDown['W'])
    {
        camera.pos.x = camera.pos.x - dt * travelSpeed * sin(camera.yaw);
        camera.pos.z = camera.pos.z - dt * travelSpeed * cos(camera.yaw);
    }

    if (ImGui::GetIO().KeysDown['S'])
    {
        camera.pos.x = camera.pos.x + dt * travelSpeed * sin(camera.yaw);
        camera.pos.z = camera.pos.z + dt * travelSpeed * cos(camera.yaw);
    }

    if (ImGui::GetIO().KeysDown['A'])
    {
        camera.pos.x = camera.pos.x - dt * travelSpeed * cos(camera.yaw);
        camera.pos.z = camera.pos.z + dt * travelSpeed * sin(camera.yaw);
    }

    if (ImGui::GetIO().KeysDown['D'])
    {
        camera.pos.x = camera.pos.x + dt * travelSpeed * cos(camera.yaw);
        camera.pos.z = camera.pos.z - dt * travelSpeed * sin(camera.yaw);
    }

    if (ImGui::GetIO().KeysDown['R'])
    {
        camera.pos.y = camera.pos.y + dt * travelSpeed;
    }

    if (ImGui::GetIO().KeysDown['F'])
    {
        camera.pos.y = camera.pos.y - dt * travelSpeed;
    }

    if (!viewport.cursorEnabled)
    {
        camera.yaw = wrap_angle(camera.yaw - (float)viewport.deltaX * rotationSpeed*dt);
        camera.pitch = mvClamp(camera.pitch -  (float)viewport.deltaY * rotationSpeed*dt, 0.995f * -MV_PI_2, 0.995f * MV_PI_2);
    }
}

mv_internal void
translate_camera(mvCamera& camera, f32 dx, f32 dy, f32 dz, f32 travelSpeed)
{
    mvVec3 direction{};
    direction.x = cos((camera.yaw)) * cos((camera.pitch));
    direction.y = sin((camera.pitch));
    direction.z = sin((camera.yaw)) * cos((camera.pitch));
    direction = mvNormalize(direction);
    camera.front = direction;

    if (dz != 0.0f)
        camera.pos = camera.pos - camera.front * travelSpeed * dz;

    if (dx != 0.0f)
        camera.pos = camera.pos - mvNormalize(mvCross(camera.front, camera.up)) * travelSpeed * dx;

    if (dy != 0.0f)
        camera.pos.y += travelSpeed * dy;
}

void
update_lookat_camera(mvViewport& viewport, mvCamera& camera, f32 dt, f32 travelSpeed, f32 rotationSpeed)
{

    if (glfwGetKey(viewport.handle, GLFW_KEY_W) == GLFW_PRESS) translate_camera(camera, 0.0f, 0.0f, dt, travelSpeed);
    if (glfwGetKey(viewport.handle, GLFW_KEY_S) == GLFW_PRESS) translate_camera(camera, 0.0f, 0.0f, -dt, travelSpeed);
    if (glfwGetKey(viewport.handle, GLFW_KEY_A) == GLFW_PRESS) translate_camera(camera, dt, 0.0f, 0.0f, travelSpeed);
    if (glfwGetKey(viewport.handle, GLFW_KEY_D) == GLFW_PRESS) translate_camera(camera, -dt, 0.0f, 0.0f, travelSpeed);
    if (glfwGetKey(viewport.handle, GLFW_KEY_R) == GLFW_PRESS) translate_camera(camera, 0.0f, dt, 0.0f, travelSpeed);
    if (glfwGetKey(viewport.handle, GLFW_KEY_F) == GLFW_PRESS) translate_camera(camera, 0.0f, -dt, 0.0f, travelSpeed);

    if (!viewport.cursorEnabled)
    {
        camera.yaw = wrap_angle(camera.yaw + (float)viewport.deltaX * rotationSpeed);
        camera.pitch = mvClamp(camera.pitch + (float)viewport.deltaY * rotationSpeed, 0.995f * -MV_PI_2, 0.995f * MV_PI_2);
    }
}