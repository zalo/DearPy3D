#include "mvSkybox.h"
#include "mvAssetManager.h"

mvSkybox
mvCreateSkybox(mvAssetManager& am)
{
    mv_local_persist i32 id = 0u;
    id++;
    std::string hash = "skybox_" + std::to_string(id);

    mvSkybox skybox{};
    skybox.descriptorSet = mvCreateDescriptorSet(am, mvGetRawDescriptorSetLayoutAsset(&am, "skybox_pass"), mvGetPipelineLayoutAssetID(&am, "skybox_pass"));
    skybox.descriptorSet.descriptors.push_back(mvCreateTextureDescriptor(am, mvCreateTextureDescriptorSpec(0u)));
    mvRegisterAsset(&am, hash, skybox.descriptorSet);

    return skybox;
}

void
mvUpdateSkyboxDescriptors(mvAssetManager& am, mvSkybox& skybox, mvAssetID texture)
{
    VkWriteDescriptorSet descriptorWrites[1];
    skybox.descriptorSet.descriptors[0].write.pImageInfo = &am.textures[texture].asset.imageInfo;

    skybox.descriptorSet.descriptors[0].write.dstSet = skybox.descriptorSet.descriptorSets[GContext->graphics.currentImageIndex];
    descriptorWrites[0] = skybox.descriptorSet.descriptors[0].write;

    vkUpdateDescriptorSets(GContext->graphics.logicalDevice, 1, descriptorWrites, 0, nullptr);
}

void
mvBindSkybox(mvAssetManager& am, mvSkybox& skybox)
{
    mvBindDescriptorSet(am, skybox.descriptorSet, 0);
}