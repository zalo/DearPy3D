#include "mvSkybox.h"
#include "mvAssetManager.h"
#include "mvGraphics.h"

mvSkybox
create_skybox(mvGraphics& graphics, mvAssetManager& am, mvMaterialManager& mManager, mvDescriptorManager& dsManager, mvPipelineManager& pmManager)
{
    mv_local_persist i32 id = 0u;
    id++;
    std::string hash = "skybox_" + std::to_string(id);

    mvSkybox skybox{};
    VkDescriptorSetLayout dsLayout = get_descriptor_set_layout(dsManager, "skybox_pass");
    VkPipelineLayout pLayout = get_pipeline_layout(pmManager, "skybox_pass");
    skybox.descriptorSet = create_descriptor_set(graphics, dsLayout, pLayout);
    skybox.descriptorSet.descriptors.push_back(create_texture_descriptor(create_texture_descriptor_spec(0u)));
    register_descriptor_set(dsManager, hash, skybox.descriptorSet);

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
    mesh.vertexBuffer = create_buffer(graphics, vertexBufferSpec, vertices.data());
    mesh.indexBuffer = create_buffer(graphics, indexBufferSpec, indices.data());

    mvMaterialData mat{};
    mat.materialColor = { 0.0f, 1.0f, 0.0f, 1.0f };
    mat.useTextureMap = true;
    mvMaterial material = create_material(graphics, dsManager, pmManager, mat, "skybox.vert.spv", "skybox.frag.spv");
    mesh.phongMaterialID = register_material(mManager, "skybox_cube_material", material);
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
bind_skybox(mvGraphics& graphics, mvSkybox& skybox)
{
    bind_descriptor_set(graphics, skybox.descriptorSet, 0);
}