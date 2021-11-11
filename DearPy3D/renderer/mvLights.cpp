#include "mvLights.h"
#include "mvContext.h"
#include "mvAssetManager.h"

mvPointLight 
mvCreatePointLight(mvAssetManager& am, mvVec3 pos)
{

    mvPointLight light;
    light.info.viewLightPos = mvVec4{ pos.x, pos.y, pos.z, 1.0f };

    for (size_t i = 0; i < GContext->graphics.swapChainImages.size(); i++)
        light.buffer.buffers.push_back(mvRegisterAsset(&am,
            "point_light",
            mvCreateDynamicBuffer(&light.info, 1, sizeof(mvPointLightInfo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)));

    mvMesh lightCube = mvCreateTexturedCube(am, 0.25f);
    auto mat1 = mvMaterialData{};
    mat1.materialColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    mat1.doLighting = false;
    
    mvMaterial material = mvCreateMaterial(am, mat1, "vs_shader.vert.spv", "ps_shader.frag.spv");
    lightCube.phongMaterialID = mvRegisterAsset(&am, "light material", material);

    mvPipelineSpec spec{};
    spec.backfaceCulling = true;
    spec.depthTest = true;
    spec.depthWrite = true;
    spec.vertexShader = "vs_shader.vert.spv";
    spec.pixelShader = "ps_shader.frag.spv";
    spec.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    am.phongMaterials[lightCube.phongMaterialID].asset.pipeline = mvGetPipelineAssetID(&am, "main_pass");
    mvRegisterAsset(&am, "light", lightCube);

    light.mesh = mvGetRawMeshAsset(&am, "light");
    return light;
}

mvDirectionLight
mvCreateDirectionLight(mvAssetManager& am, mvVec3 dir)
{

    mvDirectionLight light{};
    light.info.viewLightDir = dir;
    for (size_t i = 0; i < GContext->graphics.swapChainImages.size(); i++)
        light.buffer.buffers.push_back(mvRegisterAsset(&am,
            "direction_light",
            mvCreateDynamicBuffer(&light.info, 1, sizeof(mvDirectionLightInfo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)));
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

    mvUpdateBuffer(am.buffers[light.buffer.buffers[GContext->graphics.currentImageIndex]].asset, &light.info);

    light.info.viewLightPos = posCopy;
}

void
mvBind(mvAssetManager& am, mvDirectionLight& light, mvMat4 viewMatrix)
{

    mvVec3 posCopy = light.info.viewLightDir;

    mvVec4 out = viewMatrix * mvVec4{
        light.info.viewLightDir.x,
        light.info.viewLightDir.y,
        light.info.viewLightDir.z,
        0.0f };
    light.info.viewLightDir.x = out.x;
    light.info.viewLightDir.y = out.y;
    light.info.viewLightDir.z = out.z;

    mvUpdateBuffer(am.buffers[light.buffer.buffers[GContext->graphics.currentImageIndex]].asset, &light.info);

    light.info.viewLightDir = posCopy;
}
