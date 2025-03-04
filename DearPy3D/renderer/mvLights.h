#pragma once

#include "mvMath.h"
#include "mvMesh.h"
#include "mvBuffer.h"
#include "mvTypes.h"

// forward declarations
struct mvGraphics;
struct mvRendererContext;

struct mvPointLightInfo
{

    mvVec4 worldPos = { 0.0f, 0.0f, 0.0f, 1.0f };
    //-------------------------- ( 16 bytes )

    mvVec3 diffuseColor = { 1.0f, 1.0f, 1.0f };
    float diffuseIntensity = 1.0f;
    //-------------------------- ( 16 bytes )

    float attConst = 1.0f;
    float attLin   = 0.045f;
    float attQuad  = 0.0075f;
    char _pad1[4];
    //-------------------------- ( 16 bytes )

    mvVec4 viewPos = { 0.0f, 0.0f, 0.0f, 1.0f };
    //-------------------------- ( 4*16 = 64 bytes )
};

struct mvDirectionLightInfo
{

    mvVec3 viewLightDir = { 0.0f, -1.0f, 0.0f };
    f32    diffuseIntensity = 1.0f;
    //-------------------------- ( 16 bytes )

    mvVec3 diffuseColor = { 1.0f, 1.0f, 1.0f };
    char   _pad1[4];
    //-------------------------- ( 16 bytes )

    //-------------------------- ( 2*16 = 32 bytes )
};

struct mvPointLight
{
    mvPointLightInfo       info;
    mvMesh*                mesh = nullptr;
};

struct mvDirectionLight
{
    mvDirectionLightInfo   info;
};

mvPointLight     create_point_light      (mvRendererContext& rctx, const std::string& name, mvVec3 pos = { 0.0f,0.0f,0.5f });
mvDirectionLight create_directional_light(const std::string& name, mvVec3 dir = { 0.0f,0.0f,0.5f });
void             update_light_buffers    (mvGraphics& graphics, mvPointLight& light, mvBuffer& buffer, mvMat4 viewMatrix, u64 index);
void             update_light_buffers    (mvGraphics& graphics, mvDirectionLight& light, mvBuffer& buffer, mvMat4 viewMatrix, u64 index);
