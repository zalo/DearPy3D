#include "mvLights.h"
#include "mvGraphics.h"
#include "mvAssetManager.h"

mvPointLight 
create_point_light(mvGraphics& graphics, mvAssetManager& am, const std::string& name, mvVec3 pos)
{

    mvPointLight light;
    light.info.worldPos = mvVec4{ pos.x, pos.y, pos.z, 1.0f };

    mvMesh lightCube = create_textured_cube(graphics, am, 0.25f);
    auto mat1 = mvMaterialData{};
    mat1.materialColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    mat1.doLighting = false;
    
    mvMaterial material = create_material(graphics, am, mat1, "phong.vert.spv", "phong.frag.spv");
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
create_directional_light(mvAssetManager& am, const std::string& name, mvVec3 dir)
{

    mvDirectionLight light{};
    light.info.viewLightDir = dir;
    return light;
}

void
update_light_buffers(mvGraphics& graphics, mvAssetManager& am, mvPointLight& light, mvAssetID bufferID, mvMat4 viewMatrix, u64 index)
{
    light.info.viewPos = viewMatrix * light.info.worldPos;
    partial_buffer_update(graphics, am.buffers[bufferID].asset, &light.info, index);
}

void
update_light_buffers(mvGraphics& graphics, mvAssetManager& am, mvDirectionLight& light, mvAssetID bufferID, mvMat4 viewMatrix, u64 index)
{
    partial_buffer_update(graphics, am.buffers[bufferID].asset, &light.info, index);
}
