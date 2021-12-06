#include "mvLights.h"
#include "mvContext.h"
#include "mvAssetManager.h"

mvPointLight 
mvCreatePointLight(mvAssetManager& am, const std::string& name, mvVec3 pos)
{

    mvPointLight light;
    light.info.viewLightPos = mvVec4{ pos.x, pos.y, pos.z, 1.0f };

    mvMesh lightCube = mvCreateTexturedCube(am, 0.25f);
    auto mat1 = mvMaterialData{};
    mat1.materialColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    mat1.doLighting = false;
    
    mvMaterial material = mvCreateMaterial(am, mat1, "phong.vert.spv", "phong.frag.spv");
    lightCube.phongMaterialID = mvRegisterAsset(&am, name, material);

    mvPipelineSpec spec{};
    spec.backfaceCulling = true;
    spec.depthTest = true;
    spec.depthWrite = true;
    spec.vertexShader = "phong.vert.spv";
    spec.pixelShader = "phong.frag.spv";
    spec.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    mvRegisterAsset(&am, name, lightCube);

    light.mesh = mvGetRawMeshAsset(&am, name);
    return light;
}

mvDirectionLight
mvCreateDirectionLight(mvAssetManager& am, const std::string& name, mvVec3 dir)
{

    mvDirectionLight light{};
    light.info.viewLightDir = dir;
    return light;
}

void
mvUpdateLightBuffers(mvAssetManager& am, mvPointLight& light, mvAssetID bufferID, mvMat4 viewMatrix, u64 index)
{

    mvVec4 posCopy = light.info.viewLightPos;

    mvVec4 out = viewMatrix * light.info.viewLightPos;
    light.info.viewLightPos.x = out.x;
    light.info.viewLightPos.y = out.y;
    light.info.viewLightPos.z = out.z;

    mvPartialUpdateBuffer(am.buffers[bufferID].asset, &light.info, index);

    light.info.viewLightPos = posCopy;
}

void
mvUpdateLightBuffers(mvAssetManager& am, mvDirectionLight& light, mvAssetID bufferID, mvMat4 viewMatrix, u64 index)
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

    mvPartialUpdateBuffer(am.buffers[bufferID].asset, &light.info, index);

    light.info.viewLightDir = posCopy;
}
