#pragma once

#include "mvMath.h"
#include "mvMesh.h"
#include "mvBuffer.h"

namespace DearPy3D {

    struct PointLightInfo
    {

        glm::vec4 viewLightPos = { 0.0f, 0.0f, 0.0f, 1.0f };
        //-------------------------- ( 16 bytes )

        glm::vec3 diffuseColor = { 1.0f, 1.0f, 1.0f };
        float diffuseIntensity = 1.0f;
        //-------------------------- ( 16 bytes )

        float attConst = 1.0f;
        float attLin   = 0.045f;
        float attQuad  = 0.0075f;
        char _pad1[4];
        //-------------------------- ( 16 bytes )

        //-------------------------- ( 4*16 = 64 bytes )
    };

    struct mvPointLight
    {
        mvMesh                mesh;
        std::vector<mvBuffer> buffer;
        PointLightInfo        info = {};
    };

    mvPointLight mvCreatePointLight (glm::vec3 pos = { 0.0f,0.0f,0.5f });
    void         mvBind             (mvPointLight& light, glm::mat4 view);
    void         mvCleanupPointLight(mvPointLight& light);

}
