#include "mvLights.h"
#include "mvGraphics.h"
#include "mvRenderer.h"
#include "mvAssetLoader.h"

mvPointLight 
create_point_light(mvRendererContext& rctx, const std::string& name, mvVec3 pos)
{

    mvPointLight light;
    light.info.worldPos = mvVec4{ pos.x, pos.y, pos.z, 1.0f };

    rctx.meshes[rctx.meshCount] = create_textured_cube(*rctx.graphics, rctx, 0.25f);
    auto mat1 = mvMaterialData{};
    mat1.materialColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    mat1.doLighting = false;
    
    mvMaterial material = create_material(*rctx.graphics, rctx, mat1, "phong.vert.spv", "phong.frag.spv");
    rctx.meshes[rctx.meshCount].phongMaterialID = register_material(rctx.materialManager, name, material);

    mvPipelineSpec spec{};
    spec.backfaceCulling = true;
    spec.depthTest = true;
    spec.depthWrite = true;
    spec.vertexShader = "phong.vert.spv";
    spec.pixelShader = "phong.frag.spv";
    spec.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    light.mesh = &rctx.meshes[rctx.meshCount];
    rctx.meshCount++;
    return light;
}

mvDirectionLight
create_directional_light(const std::string& name, mvVec3 dir)
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
