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

using namespace DearPy3D;

int main() 
{
    CreateContext();
    
    Renderer::mvStartRenderer();

    mvCamera camera{};
    camera.pos = glm::vec3{5.0f, 5.0f, -15.0f};
    camera.aspect = GContext->viewport.width/GContext->viewport.height;
    
    mvMesh quad1 = mvCreateTexturedQuad("../../Resources/brickwall.jpg");
    quad1.pos.x = 5.0f;
    quad1.pos.y = 5.0f;
    quad1.pos.z = 5.0f;
    mvMesh cube1 = mvCreateTexturedCube("../../Resources/brickwall.jpg");
    cube1.pos.x = 10.0f;
    cube1.pos.y = 10.0f;
    cube1.pos.z = 10.0f;
    mvMesh cube2 = mvCreateTexturedCube("../../Resources/brickwall.jpg");
    cube2.pos.x = -10.0f;
    cube2.pos.y = 10.0f;
    cube2.pos.z = 10.0f;

    auto mat1 = mvMaterialData{};
    auto mat2 = mvMaterialData{};
    auto mat3 = mvMaterialData{};
    mat1.materialColor = glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f };
    mat2.materialColor = glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f };
    mat3.materialColor = glm::vec4{ 0.0f, 0.0f, 1.0f, 1.0f };

    mvMaterial material = mvCreateMaterial({mat1, mat2, mat3});

    //---------------------------------------------------------------------
    // main loop
    //---------------------------------------------------------------------
    mvTimer timer;

    while (GContext->viewport.running)
    {
        const auto dt = timer.mark() * 1.0f;

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
            mvCleanupMaterial(material);
            mvCleanupMesh(cube1);
            mvCleanupMesh(cube2);
            mvCleanupMesh(quad1);
            GContext->viewport.width = newwidth;
            GContext->viewport.height = newheight;
            Renderer::mvResize();

            // recreation
            auto newquad1 = mvCreateTexturedQuad("../../Resources/brickwall.jpg");
            auto newcube1 = mvCreateTexturedCube("../../Resources/brickwall.jpg");
            auto newcube2 = mvCreateTexturedCube("../../Resources/brickwall.jpg");
            
            quad1.indexBuffer = newquad1.indexBuffer;
            quad1.vertexBuffer = newquad1.vertexBuffer;
            cube1.indexBuffer = newcube1.indexBuffer;
            cube1.vertexBuffer = newcube1.vertexBuffer;
            cube2.indexBuffer = newcube2.indexBuffer;
            cube2.vertexBuffer = newcube2.vertexBuffer;

            material = mvCreateMaterial({ mat1, mat2, mat3 });

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

        glm::mat4 viewMatrix = mvBuildCameraMatrix(camera);
        glm::mat4 projMatrix = mvBuildProjectionMatrix(camera);

        Renderer::mvRenderMesh(cube1, material.pipeline, 0, {}, viewMatrix, projMatrix);
        Renderer::mvRenderMesh(cube2, material.pipeline, 1, {}, viewMatrix, projMatrix);
        Renderer::mvRenderMesh(quad1, material.pipeline, 2, {}, viewMatrix, projMatrix);

        Renderer::mvEndPass(currentCommandBuffer);

        //---------------------------------------------------------------------
        // submit command buffers & present
        //---------------------------------------------------------------------
        Renderer::mvEndFrame();
        Renderer::mvPresent();
    }

    mvCleanupMaterial(material);
    mvCleanupMesh(cube1);
    mvCleanupMesh(cube2);
    mvCleanupMesh(quad1);
    Renderer::mvStopRenderer();
    DestroyContext();
}

