#pragma once

#include "mvMath.h"
#include "mvMesh.h"
#include "mvBuffer.h"
#include "mvTypes.h"

struct mvAssetManager;

struct mvPointLightInfo
{

    mvVec4 viewLightPos = { 0.0f, 0.0f, 0.0f, 1.0f };
    //-------------------------- ( 16 bytes )

    mvVec3 diffuseColor = { 1.0f, 1.0f, 1.0f };
    float diffuseIntensity = 1.0f;
    //-------------------------- ( 16 bytes )

    float attConst = 1.0f;
    float attLin   = 0.045f;
    float attQuad  = 0.0075f;
    char _pad1[4];
    //-------------------------- ( 16 bytes )

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
    std::vector<mvAssetID> buffers;
    mvPointLightInfo       info;
    mvMesh*                mesh = nullptr;
};

struct mvDirectionLight
{
    std::vector<mvAssetID> buffers;
    mvDirectionLightInfo   info;
};

mvPointLight     mvCreatePointLight (mvAssetManager& am, const std::string& name, mvVec3 pos = { 0.0f,0.0f,0.5f });
mvDirectionLight mvCreateDirectionLight(mvAssetManager& am, const std::string& name, mvVec3 dir = { 0.0f,0.0f,0.5f });
void             mvBind             (mvAssetManager& am, mvPointLight& light, mvMat4 viewMatrix);
void             mvBind             (mvAssetManager& am, mvDirectionLight& light, mvMat4 viewMatrix);
