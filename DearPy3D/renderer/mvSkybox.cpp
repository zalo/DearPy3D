#include "mvSkybox.h"
#include "mvAssetManager.h"
#include "mvGraphics.h"

mvSkybox
create_skybox(mvGraphics& graphics, mvAssetManager& am)
{
    mv_local_persist i32 id = 0u;
    id++;
    std::string hash = "skybox_" + std::to_string(id);

    mvSkybox skybox{};
    skybox.descriptorSet = create_descriptor_set(graphics, am, mvGetRawDescriptorSetLayoutAsset(&am, "skybox_pass"), mvGetPipelineLayoutAssetID(&am, "skybox_pass"));
    skybox.descriptorSet.descriptors.push_back(create_texture_descriptor(am, create_texture_descriptor_spec(0u)));
    mvRegisterAsset(&am, hash, skybox.descriptorSet);

    const float side = 1.0f / 2.0f;
    auto vertices = std::vector<float>{
        -side, -side, -side,
         side, -side, -side,
        -side,  side, -side,
         side,  side, -side,
        -side, -side,  side,
         side, -side,  side,
        -side,  side,  side,
         side,  side,  side
    };

    mv_local_persist auto indices = std::vector<uint32_t>{
        0, 2, 1, 2, 3, 1,
        1, 3, 5, 3, 7, 5,
        2, 6, 3, 3, 6, 7,
        4, 5, 7, 4, 7, 6,
        0, 4, 2, 2, 4, 6,
        0, 1, 4, 1, 5, 4
    };

    mvBufferSpecification vertexBufferSpec{};
    vertexBufferSpec.usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBufferSpec.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    vertexBufferSpec.size = sizeof(f32);
    vertexBufferSpec.components = 14;
    vertexBufferSpec.count = vertices.size();

    mvBufferSpecification indexBufferSpec{};
    indexBufferSpec.usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBufferSpec.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    indexBufferSpec.size = sizeof(u32);
    indexBufferSpec.count = indices.size();

    mvMesh mesh{};
    mesh.name = "skybox cube";
    mesh.vertexBuffer = mvRegisterAsset(&am, "skybox_cube_vertex",
        create_buffer(graphics, vertexBufferSpec, vertices.data()));
    if (mvGetBufferAssetID(&am, "skybox_cube_index") == MV_INVALID_ASSET_ID)
        mesh.indexBuffer = mvRegisterAsset(&am, "skybox_cube_index",
            create_buffer(graphics, indexBufferSpec, indices.data()));
    else
        mesh.indexBuffer = mvGetBufferAssetID(&am, "skybox_cube_index");

    mvMaterialData mat{};
    mat.materialColor = { 0.0f, 1.0f, 0.0f, 1.0f };
    mat.useTextureMap = true;
    mvMaterial material = create_material(graphics, am, mat, "skybox.vert.spv", "skybox.frag.spv");
    mesh.phongMaterialID = mvRegisterAsset(&am, "skybox_cube_material", material);
    mesh.diffuseTexture = mvGetTextureAssetID2(graphics, &am, "../../Resources/SkyBox");

    skybox.mesh = mesh;

    return skybox;
}

void
update_skybox_descriptors(mvGraphics& graphics, mvAssetManager& am, mvSkybox& skybox, mvAssetID texture)
{
    VkWriteDescriptorSet descriptorWrites[1];
    skybox.descriptorSet.descriptors[0].write.pImageInfo = &am.textures[texture].asset.imageInfo;

    skybox.descriptorSet.descriptors[0].write.dstSet = skybox.descriptorSet.descriptorSets[graphics.currentImageIndex];
    descriptorWrites[0] = skybox.descriptorSet.descriptors[0].write;

    vkUpdateDescriptorSets(graphics.logicalDevice, 1, descriptorWrites, 0, nullptr);
}

void
bind_skybox(mvGraphics& graphics, mvAssetManager& am, mvSkybox& skybox)
{
    bind_descriptor_set(graphics, am, skybox.descriptorSet, 0);
}