#include "mvLights.h"
#include "mvContext.h"
#include "mvAssetManager.h"

mvPointLight 
mvCreatePointLight(mvAssetManager& am, mvVec3 pos)
{

    mvPointLight light;
    light.info.viewLightPos = mvVec4{ pos.x, pos.y, pos.z, 1.0f };

    for (size_t i = 0; i < GContext->graphics.swapChainImages.size(); i++)
        light.buffer.buffers.push_back(mvGetDynamicBufferAsset(&am,
            &light.info, 
            1, 
            sizeof(mvPointLightInfo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, "point_light"));

    return light;
}

void
mvBind(mvAssetManager& am, mvPointLight& light, mvMat4 viewMatrix)
{

    mvVec4 posCopy = light.info.viewLightPos;

    mvVec4 out = viewMatrix * light.info.viewLightPos;
    light.info.viewLightPos.x = out.x;
    light.info.viewLightPos.y = out.y;
    light.info.viewLightPos.z = out.z;

    mvUpdateBuffer(am.dynBuffers[light.buffer.buffers[GContext->graphics.currentImageIndex]].buffer, &light.info);

    //vkCmdBindDescriptorSets(mvGetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
    //    pipelineLayout, 0, 1, &light.descriptorSets[GContext->graphics.currentImageIndex], 0, nullptr);

    light.info.viewLightPos = posCopy;
}
