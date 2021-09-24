#pragma once

#include "mvMath.h"
#include "mvMesh.h"
#include "mvBuffer.h"

namespace DearPy3D {

    struct mvPointLightInfo
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

        char _pad2[192];
        //-------------------------- ( 192 bytes )

        //-------------------------- ( 4*16 + 192 = 256 bytes )
    };

    struct mvPointLightBuffer
    {
        std::vector<mvBuffer> buffers;
    };

    struct mvPointLight
    {
        mvPointLightBuffer    buffer;
        VkDescriptorSet       descriptorSets[MV_MAX_FRAMES_IN_FLIGHT+1];
        VkDescriptorSetLayout descriptorSetLayout;
        mvPointLightInfo      info;
    };

    mvPointLight mvCreatePointLight (glm::vec3 pos = { 0.0f,0.0f,0.5f });
    void         mvBind             (mvPointLight& light, glm::mat4 viewMatrix, VkPipelineLayout pipelineLayout);
    void         mvCleanupPointLight(mvPointLight& light);

}
