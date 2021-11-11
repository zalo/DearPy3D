#include "mvMesh.h"
#include "mvContext.h"
#include "mvObjLoader.h"
#include "mvAssetManager.h"

mvMesh 
mvCreateTexturedCube(mvAssetManager& assetManager, float sideLength)
{

    auto vertices = std::vector<float>{
        -sideLength, -sideLength, -sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 0 near side
         sideLength, -sideLength, -sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 1
        -sideLength,  sideLength, -sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 2
         sideLength,  sideLength, -sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 3

        -sideLength, -sideLength,  sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 4 far side
         sideLength, -sideLength,  sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 5
        -sideLength,  sideLength,  sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 6
         sideLength,  sideLength,  sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 7

        -sideLength, -sideLength, -sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 8 left side
        -sideLength,  sideLength, -sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 9
        -sideLength, -sideLength,  sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 10
        -sideLength,  sideLength,  sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 11

         sideLength, -sideLength, -sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 12 right side
         sideLength,  sideLength, -sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 13
         sideLength, -sideLength,  sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 14
         sideLength,  sideLength,  sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 15

        -sideLength, -sideLength, -sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 16 bottom side
         sideLength, -sideLength, -sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 17
        -sideLength, -sideLength,  sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 18
         sideLength, -sideLength,  sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 19

        -sideLength,  sideLength, -sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 20 top side
         sideLength,  sideLength, -sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 21
        -sideLength,  sideLength,  sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 22
         sideLength,  sideLength,  sideLength, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f   // 23
    };

    mv_local_persist auto indices = std::vector<uint32_t>{
        1,  2,  0,  1,  3,  2,
        5,  4,  7,  7,  4,  6,
        10, 9,  11, 10, 8, 9,
        13, 12, 15, 15, 12, 14,
        18, 17, 16, 18, 19, 17,
        23, 20, 21, 22, 20, 23
    };

    // calculate normals
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        auto v0 = vertices[14 * indices[i]];
        auto v1 = vertices[14 * indices[i + 1]];
        auto v2 = vertices[14 * indices[i + 2]];
        mvVec3 p0 = { v0, vertices[14 * indices[i] + 1], vertices[14 * indices[i] + 2] };
        mvVec3 p1 = { v1, vertices[14 * indices[i + 1] + 1], vertices[14 * indices[i + 1] + 2] };
        mvVec3 p2 = { v2, vertices[14 * indices[i + 2] + 1], vertices[14 * indices[i + 2] + 2] };

        mvVec3 n = mvNormalize(mvCross(p1 - p0, p2 - p0));
        vertices[14 * indices[i] + 3] = n[0];
        vertices[14 * indices[i] + 4] = n[1];
        vertices[14 * indices[i] + 5] = n[2];
        vertices[14 * indices[i + 1] + 3] = n[0];
        vertices[14 * indices[i + 1] + 4] = n[1];
        vertices[14 * indices[i + 1] + 5] = n[2];
        vertices[14 * indices[i + 2] + 3] = n[0];
        vertices[14 * indices[i + 2] + 4] = n[1];
        vertices[14 * indices[i + 2] + 5] = n[2];
    }

    mvMesh mesh{};
    mesh.name = "textured cube";
    mesh.vertexBuffer = mvRegisterAsset(&assetManager, "textured_cube_vertex" + std::to_string(sideLength),
        mvCreateBuffer(vertices.data(), vertices.size(), sizeof(u32), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
    if(mvGetBufferAssetID(&assetManager, "textured_cube_index") == MV_INVALID_ASSET_ID)
        mesh.indexBuffer = mvRegisterAsset(&assetManager, "textured_cube_index",
            mvCreateBuffer(indices.data(), indices.size(), sizeof(u32), VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
    else
        mesh.indexBuffer = mvGetBufferAssetID(&assetManager, "textured_cube_index");
    
    mvMaterialData mat{};
    mat.materialColor = { 0.0f, 1.0f, 0.0f, 1.0f };
    mat.useTextureMap = true;
    mvMaterial material = mvCreateMaterial(assetManager, mat, "vs_shader.vert.spv", "ps_shader.frag.spv");
    mesh.phongMaterialID = mvRegisterAsset(&assetManager, "textured_cube_material", material);
    mesh.diffuseTexture = mvGetTextureAssetID(&assetManager, "../../Resources/brickwall.jpg");
    return mesh;
}

mvMesh 
mvCreateTexturedQuad(mvAssetManager& assetManager, float sideLength)
{

    // initialize vertices
    auto vertices = std::vector<float>
    {
        -sideLength,  sideLength, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         sideLength, -sideLength, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -sideLength, -sideLength, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
         sideLength,  sideLength, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    // 0 O----O 3
    //   |\   |
    //   | \  |
    //   |  \ |
    //   |   \|
    // 2 O----O 1

    // initialize indexes
    auto indices = std::vector<uint32_t>{ 0u, 2u, 1u, 3u, 0u, 1u };

    // calculate normals
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        const float v0 = vertices[14 * indices[i]];
        const float v1 = vertices[14 * indices[i + 1]];
        const float v2 = vertices[14 * indices[i + 2]];
        mvVec3 p0 = { v0, vertices[14 * indices[i] + 1],     vertices[14 * indices[i] + 2] };
        mvVec3 p1 = { v1, vertices[14 * indices[i + 1] + 1], vertices[14 * indices[i + 1] + 2] };
        mvVec3 p2 = { v2, vertices[14 * indices[i + 2] + 1], vertices[14 * indices[i + 2] + 2] };

        mvVec3 n = mvNormalize(mvCross(p1 - p0, p2 - p0));
        vertices[14 * indices[i] + 3] = n[0];
        vertices[14 * indices[i] + 4] = n[1];
        vertices[14 * indices[i] + 5] = n[2];
        vertices[14 * indices[i + 1] + 3] = n[0];
        vertices[14 * indices[i + 1] + 4] = n[1];
        vertices[14 * indices[i + 1] + 5] = n[2];
        vertices[14 * indices[i + 2] + 3] = n[0];
        vertices[14 * indices[i + 2] + 4] = n[1];
        vertices[14 * indices[i + 2] + 5] = n[2];
    }

    mvMesh mesh{};
    mesh.name = "textured quad";
    mesh.vertexBuffer = mvRegisterAsset(&assetManager, "textured_cube_vertex" + std::to_string(sideLength),
        mvCreateBuffer(vertices.data(), vertices.size(), sizeof(u32), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
    if (mvGetBufferAssetID(&assetManager, "textured_quad_vertex") == MV_INVALID_ASSET_ID)
        mesh.indexBuffer = mvRegisterAsset(&assetManager, "textured_quad_index",
            mvCreateBuffer(indices.data(), indices.size(), sizeof(u32), VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
    else
        mesh.indexBuffer = mvGetBufferAssetID(&assetManager, "textured_quad_index");

    mvMaterialData mat{};
    mat.materialColor = { 1.0f, 0.0f, 0.0f, 1.0f };
    mat.useTextureMap = true;
    mvMaterial material = mvCreateMaterial(assetManager, mat, "vs_shader.vert.spv", "ps_shader.frag.spv");
    mesh.phongMaterialID = mvRegisterAsset(&assetManager, "textured_cube_material", material);
    mesh.diffuseTexture = mvGetTextureAssetID(&assetManager, "../../Resources/brickwall.jpg");
    return mesh;
}

void
mvLoadOBJAssets(mvAssetManager& assetManager, const std::string& root, const std::string& file)
{
    u32 nodeOffset = assetManager.nodeCount;
    u32 meshOffset = assetManager.meshCount;

    std::vector<mvAssetID> diffuseTextureMaps;
    std::vector<mvAssetID> normalTextureMaps;
    std::vector<mvAssetID> specularTextureMaps;
    std::vector<mvObjMaterial> objMaterials = mvLoadObjMaterials(root + file + ".mtl");
    mvObjModel objModel = mvLoadObjModel(root + file + ".obj");

    for (size_t i = 0; i < objModel.meshes.size(); i++)
    {
        for (size_t j = 0; j < objMaterials.size(); j++)
        {
            if (objMaterials[j].name == objModel.meshes[i]->material)
            {
                mvMesh newMesh{};
                mvMaterialData materialData{};
                newMesh.pbr = false;
                newMesh.name = objModel.meshes[i]->name;

                if (!objMaterials[j].diffuseMap.empty())
                {
                    newMesh.diffuseTexture = mvGetTextureAssetID(&assetManager, root + objMaterials[j].diffuseMap);
                    materialData.useTextureMap = true;
                    materialData.hasAlpha = true;
                }
                if (!objMaterials[j].normalMap.empty())
                {
                    newMesh.normalTexture = mvGetTextureAssetID(&assetManager, root + objMaterials[j].normalMap);
                    materialData.useNormalMap = true;
                }
                if (!objMaterials[j].specularMap.empty())
                {
                    newMesh.specularTexture = mvGetTextureAssetID(&assetManager, root + objMaterials[j].specularMap);
                    materialData.useSpecularMap = true;
                }

                newMesh.vertexBuffer = mvRegisterAsset(&assetManager, newMesh.name + "_vertex", mvCreateBuffer(objModel.meshes[i]->averticies.data(), objModel.meshes[i]->averticies.size(), sizeof(f32) * 14, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
                newMesh.indexBuffer = mvRegisterAsset(&assetManager, newMesh.name + "_index", mvCreateBuffer(objModel.meshes[i]->indicies.data(), objModel.meshes[i]->indicies.size(), sizeof(u32), VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
                
                mvMaterial material = mvCreateMaterial(assetManager, materialData, "vs_shader.vert.spv", "ps_shader.frag.spv");
                newMesh.phongMaterialID = mvRegisterAsset(&assetManager, newMesh.name + "material", material);

                mvRegisterAsset(&assetManager, file + std::to_string(i), newMesh);
            }
        }

    }

    mvScene newScene = mvCreateScene(assetManager, {});
    newScene.nodeCount = objModel.meshes.size();
    for (i32 i = 0; i < newScene.nodeCount; i++)
    {
        newScene.nodes[i] = i + nodeOffset;
    }
    mvRegisterAsset(&assetManager, file, newScene);

    for (u32 currentNode = 0u; currentNode < objModel.meshes.size(); currentNode++)
    {
        mvNode newNode{};
        newNode.name = objModel.meshes[currentNode]->name;
        newNode.mesh = newScene.nodes[currentNode] - nodeOffset + meshOffset;
        newNode.childCount = 0;
        mvRegisterAsset(&assetManager, file + std::to_string(currentNode), newNode);
    }


}