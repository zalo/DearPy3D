#include "mvPointLight.h"
#include "mvContext.h"
#include "mvGraphicsContext.h"

namespace DearPy3D {


    mvPointLight mvCreatePointLight(glm::vec3 pos)
    {

        mvPointLight light;
        light.mesh = mvCreateSolidSphere();
        light.mesh.pos = pos;
        light.info.viewLightPos = glm::vec4(pos, 1.0f);

        for (int i = 0; i < 3; i++)
            light.buffer.push_back(mvCreateBuffer<PointLightInfo>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));  
    
        return light;
    }

    void mvBind(mvPointLight& light, glm::mat4 view)
    {
        glm::vec4 posCopy = light.info.viewLightPos;

        glm::vec3 out = view * light.info.viewLightPos;
        light.info.viewLightPos.x = out.x;
        light.info.viewLightPos.y = out.y;
        light.info.viewLightPos.z = out.z;

        mvUpdateBuffer(light.buffer[GContext->graphics.currentImageIndex], light.info);

        light.info.viewLightPos = posCopy;
    }

    void mvCleanupPointLight(mvPointLight& light)
    {
        for (auto& item : light.buffer)
            mvCleanupBuffer(item);
        mvCleanupDrawable(light.mesh);
    }
}
