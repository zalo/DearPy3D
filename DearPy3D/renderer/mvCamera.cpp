#include "mvCamera.h"
#include <imgui.h>
#include <cmath>
#include "sSemper.h"
#include "mvMath.h"

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

void
update_fps_camera(sWindow& viewport, mvCamera& camera, f32 dt, f32 travelSpeed, f32 rotationSpeed)
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

    static bool justEnabled = true;
    if (!viewport.cursorEnabled)
    {
        if (!justEnabled)
        {
            ImGuiIO& io = ImGui::GetIO();
            camera.yaw = wrap_angle(camera.yaw - (float)io.MouseDelta.x * rotationSpeed * dt);
            camera.pitch = mvMath::clamp(camera.pitch - (float)io.MouseDelta.y * rotationSpeed * dt, 0.995f * -MV_PI_2, 0.995f * MV_PI_2);
        }
        justEnabled = false;
    }
    else
        justEnabled = true;
}