#include "mvAssetLoader.h"
#include "mvRenderer.h"
#include "mvObjLoader.h"

mvModel
load_obj_model(mvRendererContext& rctx, const std::string& root, const std::string& file)
{
    mvModel model{};

    std::vector<mvAssetID> diffuseTextureMaps;
    std::vector<mvAssetID> normalTextureMaps;
    std::vector<mvAssetID> specularTextureMaps;
    std::vector<mvObjMaterial> objMaterials = mvLoadObjMaterials(root + file + ".mtl");
    mvObjModel objModel = mvLoadObjModel(root + file + ".obj");

    model.meshCount = objModel.meshes.size();
    model.nodeCount = objModel.meshes.size();
    model.sceneCount = 1;
    model.meshes = new mvMesh[model.meshCount];
    model.scenes = new mvScene[model.sceneCount];
    model.nodes = new mvNode[model.nodeCount];

    for (size_t i = 0; i < model.meshCount; i++)
    {
        for (size_t j = 0; j < objMaterials.size(); j++)
        {
            if (objMaterials[j].name == objModel.meshes[i]->material)
            {
                mvMaterialData materialData{};
                model.meshes[i].name = objModel.meshes[i]->name;

                if (!objMaterials[j].diffuseMap.empty())
                {
                    model.meshes[i].diffuseTexture = register_texture_safe_load(*rctx.graphics, rctx.textureManager, root + objMaterials[j].diffuseMap);
                    materialData.useTextureMap = true;
                    materialData.hasAlpha = true;
                }
                if (!objMaterials[j].normalMap.empty())
                {
                    model.meshes[i].normalTexture = register_texture_safe_load(*rctx.graphics, rctx.textureManager, root + objMaterials[j].normalMap);
                    materialData.useNormalMap = true;
                }
                if (!objMaterials[j].specularMap.empty())
                {
                    model.meshes[i].specularTexture = register_texture_safe_load(*rctx.graphics, rctx.textureManager, root + objMaterials[j].specularMap);
                    materialData.useSpecularMap = true;
                }

                mvBufferSpecification vertexBufferSpec{};
                vertexBufferSpec.usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                vertexBufferSpec.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                vertexBufferSpec.size = sizeof(f32);
                vertexBufferSpec.components = 14;
                vertexBufferSpec.count = objModel.meshes[i]->averticies.size();

                mvBufferSpecification indexBufferSpec{};
                indexBufferSpec.usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
                indexBufferSpec.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                indexBufferSpec.size = sizeof(u32);
                indexBufferSpec.count = objModel.meshes[i]->indicies.size();

                model.meshes[i].vertexBuffer = create_buffer(*rctx.graphics, vertexBufferSpec, objModel.meshes[i]->averticies.data());
                model.meshes[i].indexBuffer = create_buffer(*rctx.graphics, indexBufferSpec, objModel.meshes[i]->indicies.data());

                mvMaterial material = create_material(*rctx.graphics, rctx, materialData, "phong.vert.spv", "phong.frag.spv");
                model.meshes[i].phongMaterialID = register_material(rctx.materialManager, model.meshes[i].name + "material", material);


            }
        }

    }

    model.scenes[0] = create_scene(rctx, {});
    model.scenes[0].nodeCount = objModel.meshes.size();

    for (i32 i = 0; i < model.scenes[0].nodeCount; i++)
    {
        model.scenes[0].nodes[i] = i;
    }

    for (u32 currentNode = 0u; currentNode < objModel.meshes.size(); currentNode++)
    {
        model.nodes[currentNode].name = objModel.meshes[currentNode]->name;
        model.nodes[currentNode].mesh = model.scenes[0].nodes[currentNode];
        model.nodes[currentNode].childCount = 0;
    }

    return model;
}