#pragma once

#include "mvMath.h"
#include "mvBuffer.h"
#include "mvPipeline.h"
#include "mvMaterials.h"
#include "mvTypes.h"

// forward declarations
struct mvObjMesh;
struct mvAssetManager;
struct mvGraphics;
struct mvMesh;

mvMesh create_textured_cube(mvGraphics& graphics, mvAssetManager& assetManager, mvDescriptorManager& dsManager, mvPipelineManager& pmManager, mvMaterialManager& mManager, float sideLength = 1.0f);
mvMesh create_textured_quad(mvGraphics& graphics, mvAssetManager& assetManager, mvDescriptorManager& dsManager, mvPipelineManager& pmManager, mvMaterialManager& mManager, float sideLength = 1.0f);
void   load_obj_assets     (mvGraphics& graphics, mvAssetManager& assetManager, mvDescriptorManager& dsManager, mvPipelineManager& pmManager, mvMaterialManager& mManager, const std::string& root, const std::string& file);

struct mvMesh
{
    std::string name;
    mvBuffer    indexBuffer;
    mvBuffer    vertexBuffer;
    mvAssetID   diffuseTexture = -1;
    mvAssetID   normalTexture = -1;
    mvAssetID   specularTexture = -1;
    mvAssetID   albedoTexture = -1;
    mvAssetID   metalRoughnessTexture = -1;
    mvAssetID   phongMaterialID = -1;
};
