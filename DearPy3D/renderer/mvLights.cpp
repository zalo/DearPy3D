#include "mvLights.h"
#include "mvGraphics.h"
#include "mvAssetManager.h"

mvPointLight 
create_point_light(mvGraphics& graphics, mvAssetManager& am, mvDescriptorManager& dsManager, mvPipelineManager& pmManager, mvMaterialManager& mManager, const std::string& name, mvVec3 pos)
{

    mvPointLight light;
    light.info.worldPos = mvVec4{ pos.x, pos.y, pos.z, 1.0f };

    mvMesh lightCube = create_textured_cube(graphics, am, dsManager, pmManager, mManager, 0.25f);
    auto mat1 = mvMaterialData{};
    mat1.materialColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    mat1.doLighting = false;
    
    mvMaterial material = create_material(graphics, dsManager, pmManager, mat1, "phong.vert.spv", "phong.frag.spv");
    lightCube.phongMaterialID = register_material(mManager, name, material);

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
update_light_buffers(mvGraphics& graphics, mvPointLight& light, mvBuffer& buffer, mvMat4 viewMatrix, u64 index)
{
    light.info.viewPos = viewMatrix * light.info.worldPos;
    partial_buffer_update(graphics, buffer, &light.info, index);
}

void
update_light_buffers(mvGraphics& graphics, mvDirectionLight& light, mvBuffer& buffer, mvMat4 viewMatrix, u64 index)
{
    partial_buffer_update(graphics, buffer, &light.info, index);
}
