#include <imgui.h>
#include "mvContext.h"
#include "mvMesh.h"
#include "mvCamera.h"
#include "mvTimer.h"
#include "mvPointLight.h"
#include "mvMaterial.h"
#include "mvContext.h"
#include <iostream>
#include "mvMath.h"
#include "mvRenderer.h"
#include "mvObjLoader.h"

mv_internal bool loadModel = false;

int main() 
{
    CreateContext();
    
    Renderer::mvStartRenderer();

    mvCamera camera{};
    camera.pos = {5.0f, 5.0f, -15.0f};
    camera.aspect = GContext->viewport.width/GContext->viewport.height;
    
    mvMesh quad1 = mvCreateTexturedQuad();
    quad1.pos.x = 5.0f;
    quad1.pos.y = 5.0f;
    quad1.pos.z = 5.0f;
    mvMesh cube1 = mvCreateTexturedCube(3.0f);
    cube1.pos.x = 10.0f;
    cube1.pos.y = 10.0f;
    cube1.pos.z = 20.0f;
    mvMesh lightCube = mvCreateTexturedCube(0.25f);
    lightCube.pos.y = 10.0f;

    std::vector<mvMesh> meshes;
    mvObjModel objModel{};
    if (loadModel)
    {
        objModel = mvLoadObjModel("../../Dependencies/MarvelAssets/Sponza/sponza.obj");
        for (auto mesh : objModel.meshes)
        {
            meshes.push_back(mvCreateObjMesh(*mesh));
        }
    }

    auto mat1 = mvMaterialData{};
    auto mat2 = mvMaterialData{};
    auto mat3 = mvMaterialData{};
    mat1.materialColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    mat1.doLighting = false;
    mat2.materialColor = { 0.0f, 1.0f, 0.0f, 1.0f };
    mat3.materialColor = { 1.0f, 0.0f, 0.0f, 1.0f };

    mvMaterial material1 = mvCreateMaterial(mat1, "vs_shader.vert.spv", "ps_shader.frag.spv");
    mvMaterial material2 = mvCreateMaterial(mat2, "vs_shader.vert.spv", "ps_shader.frag.spv");
    mvMaterial material3 = mvCreateMaterial(mat3, "vs_shader.vert.spv", "ps_shader.frag.spv");
    mvPointLight light = mvCreatePointLight({0.0f, 10.0f, 0.0f});

    std::vector<mvMaterial> materials;
    std::vector<mvObjMaterial> objMaterials;
    if (loadModel)
    {
        objMaterials = mvLoadObjMaterials("../../Dependencies/MarvelAssets/Sponza/sponza.mtl");
        for (auto objMaterial : objMaterials)
        {
            materials.push_back(mvCreateObjMaterial({}, objMaterial, "vs_shader.vert.spv", "ps_shader.frag.spv", "../../Dependencies/MarvelAssets/Sponza/"));
            mvFinalizePipeline(materials.back().pipeline, { materials.back().descriptorSetLayout, light.descriptorSetLayout });
        }
    }

    mvFinalizePipeline(material1.pipeline, {material1.descriptorSetLayout, light.descriptorSetLayout});
    mvFinalizePipeline(material2.pipeline, {material2.descriptorSetLayout, light.descriptorSetLayout});
    mvFinalizePipeline(material3.pipeline, {material3.descriptorSetLayout, light.descriptorSetLayout});

    //---------------------------------------------------------------------
    // main loop
    //---------------------------------------------------------------------
    mvTimer timer;

    while (GContext->viewport.running)
    {
        const auto dt = timer.mark() * 1.0f;
        //quad1.rot.z += dt;

        mvProcessViewportEvents();

        //---------------------------------------------------------------------
        // handle window resizing
        //---------------------------------------------------------------------
        if (GContext->viewport.resized)
        {

            int newwidth = 0, newheight = 0;
            glfwGetFramebufferSize(GContext->viewport.handle, &newwidth, &newheight);
            while (newwidth == 0 || newheight == 0)
            {
                glfwGetFramebufferSize(GContext->viewport.handle, &newwidth, &newheight);
                glfwWaitEvents();
            }

            camera.aspect = (float)newwidth/(float)newheight;

            // cleanup
            mvCleanupMaterial(material1);
            mvCleanupMaterial(material2);
            mvCleanupMaterial(material3);

            if (loadModel)
            {
                for (auto& material : materials)
                {
                    mvCleanupMaterial(material);
                }

                for (auto& mesh : meshes)
                {
                    mvCleanupMesh(mesh);
                }
            }

            mvCleanupPointLight(light);
            mvCleanupMesh(cube1);
            mvCleanupMesh(lightCube);
            mvCleanupMesh(quad1);
            GContext->viewport.width = newwidth;
            GContext->viewport.height = newheight;
            Renderer::mvResize();

            // recreation
            auto newquad1 = mvCreateTexturedQuad();
            auto newcube1 = mvCreateTexturedCube(3.0f);
            auto newlightcube = mvCreateTexturedCube(0.25f);
            
            quad1.indexBuffer = newquad1.indexBuffer;
            quad1.vertexBuffer = newquad1.vertexBuffer;
            cube1.indexBuffer = newcube1.indexBuffer;
            cube1.vertexBuffer = newcube1.vertexBuffer;
            lightCube.indexBuffer = newlightcube.indexBuffer;
            lightCube.vertexBuffer = newlightcube.vertexBuffer;


            if (loadModel)
            {

                objModel = mvLoadObjModel("../../Dependencies/MarvelAssets/Sponza/sponza.obj");
                for (int i = 0; i < objModel.meshes.size(); i++)
                {
                    auto newmesh = mvCreateObjMesh(*objModel.meshes[i]);
                    meshes[i].indexBuffer = newmesh.indexBuffer;
                    meshes[i].vertexBuffer = newmesh.vertexBuffer;
                }

            }

            light = mvCreatePointLight({ 0.0f, 10.0f, 0.0f });
            material1 = mvCreateMaterial(mat1, "vs_shader.vert.spv", "ps_shader.frag.spv");
            material2 = mvCreateMaterial(mat2, "vs_shader.vert.spv", "ps_shader.frag.spv");
            material3 = mvCreateMaterial(mat3, "vs_shader.vert.spv", "ps_shader.frag.spv");
            mvFinalizePipeline(material1.pipeline, { material1.descriptorSetLayout, light.descriptorSetLayout });
            mvFinalizePipeline(material2.pipeline, { material2.descriptorSetLayout, light.descriptorSetLayout });
            mvFinalizePipeline(material3.pipeline, { material3.descriptorSetLayout, light.descriptorSetLayout });

            if (loadModel)
            {
                objMaterials = mvLoadObjMaterials("../../Dependencies/MarvelAssets/Sponza/sponza.mtl");
                materials.clear();
                for (auto objMaterial : objMaterials)
                {
                    materials.push_back(mvCreateObjMaterial({}, objMaterial, "vs_shader.vert.spv", "ps_shader.frag.spv", "../../Dependencies/MarvelAssets/Sponza/"));
                    mvFinalizePipeline(materials.back().pipeline, { materials.back().descriptorSetLayout, light.descriptorSetLayout });
                }
            }

            GContext->viewport.resized = false;

            // hacky but works
            // window only seems to repaint if moved, so we move the window
            // slightly
            int x, y;
            glfwGetWindowPos(GContext->viewport.handle, &x, &y);
            glfwSetWindowPos(GContext->viewport.handle, ++x, y);
        }

        //---------------------------------------------------------------------
        // input handling
        //---------------------------------------------------------------------
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_W) == GLFW_PRESS) mvTranslateCamera(camera, 0.0f, 0.0f, dt);
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_S) == GLFW_PRESS) mvTranslateCamera(camera, 0.0f, 0.0f, -dt);
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_A) == GLFW_PRESS) mvTranslateCamera(camera, -dt, 0.0f, 0.0f);
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_D) == GLFW_PRESS) mvTranslateCamera(camera, dt, 0.0f, 0.0f);
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_R) == GLFW_PRESS) mvTranslateCamera(camera, 0.0f, dt, 0.0f);
        if (glfwGetKey(GContext->viewport.handle, GLFW_KEY_F) == GLFW_PRESS) mvTranslateCamera(camera, 0.0f, -dt, 0.0f);
        if (!GContext->viewport.cursorEnabled) mvRotateCamera(camera, GContext->viewport.deltaX, GContext->viewport.deltaY);
        
        //---------------------------------------------------------------------
        // wait for fences and acquire next image
        //---------------------------------------------------------------------
        Renderer::mvBeginFrame();

        //---------------------------------------------------------------------
        // main pass
        //---------------------------------------------------------------------

        auto currentCommandBuffer = mvGetCurrentCommandBuffer();

        Renderer::mvBeginPass(currentCommandBuffer, GContext->graphics.renderPass);

        ImGuiIO& io = ImGui::GetIO();
        ImGui::GetForegroundDrawList()->AddText(ImVec2(45, 45),
            ImColor(0.0f, 1.0f, 0.0f), std::string(std::to_string(io.Framerate) + " FPS").c_str());

        ImGui::Begin("Light Controls");
        if (ImGui::SliderFloat3("Position", &light.info.viewLightPos.x, -25.0f, 25.0f))
        {
            lightCube.pos.x = light.info.viewLightPos.x;
            lightCube.pos.y = light.info.viewLightPos.y;
            lightCube.pos.z = light.info.viewLightPos.z;
        }
        ImGui::End();

        mvMat4 viewMatrix = mvBuildCameraMatrix(camera);
        mvMat4 projMatrix = mvBuildProjectionMatrix(camera);

        mvBind(light, viewMatrix, material1.pipeline.pipelineLayout);
        Renderer::mvRenderMesh(lightCube, material1, mvTranslate(mvIdentityMat4(), mvVec3{ 0.0f, 0.0f, 0.0f }), viewMatrix, projMatrix);

        mvMat4 extratransform = mvTranslate(mvIdentityMat4(), mvVec3{ 0.0f, 0.0f, 0.0f });
        Renderer::mvRenderMesh(cube1, material2, extratransform, viewMatrix, projMatrix);
        Renderer::mvRenderMesh(quad1, material3, extratransform, viewMatrix, projMatrix);

        if (loadModel)
        {
            for (int i = 0; i < meshes.size(); i++)
            {
                for (int j = 0; j < objMaterials.size(); j++)
                {
                    if (objMaterials[j].name == objModel.meshes[i]->material)
                        Renderer::mvRenderMesh(meshes[i], materials[j], extratransform, viewMatrix, projMatrix);
                }

            }
        }
        
        Renderer::mvEndPass(currentCommandBuffer);

        //---------------------------------------------------------------------
        // submit command buffers & present
        //---------------------------------------------------------------------
        Renderer::mvEndFrame();
        Renderer::mvPresent();
    }

    mvCleanupMaterial(material1);
    mvCleanupMaterial(material2);
    mvCleanupMaterial(material3);
    mvCleanupPointLight(light);

    if (loadModel)
    {
        for (auto& material : materials)
        {
            mvCleanupMaterial(material);
        }

        for (auto& mesh : meshes)
        {
            mvCleanupMesh(mesh);
        }
    }

    mvCleanupMesh(cube1);
    mvCleanupMesh(lightCube);
    mvCleanupMesh(quad1);
    Renderer::mvStopRenderer();
    DestroyContext();
}

