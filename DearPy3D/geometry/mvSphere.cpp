#include <iostream>
#include <iomanip>
#include <cmath>
#include "mvSphere.h"

static const int MIN_SECTOR_COUNT = 3;
static const int MIN_STACK_COUNT  = 2;

namespace DearPy3D {

    static void               mvBuildVerticesSmooth     (mvSphere& sphere, int sectorCount, int stackCount);
    static void               mvBuildVerticesFlat       (mvSphere& sphere, int sectorCount, int stackCount);
    static void               mvBuildInterleavedVertices(mvSphere& sphere);
    static std::vector<float> mvComputeFaceNormal(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);

    mvSphere mvCreateSphere(float radius, int sectorCount, int stackCount, bool smooth)
    {
        mvSphere sphere;

        sphere.radius = radius;

        if (sectorCount < MIN_SECTOR_COUNT)
            sectorCount = MIN_SECTOR_COUNT;
        if (sectorCount < MIN_STACK_COUNT)
            sectorCount = MIN_STACK_COUNT;


        if (smooth)
            mvBuildVerticesSmooth(sphere, sectorCount, stackCount);
        else
            mvBuildVerticesFlat(sphere, sectorCount, stackCount);

        return sphere;
    }

    //-----------------------------------------------------------------------------
    // build vertices of sphere with smooth shading using parametric equation
    // x = r * cos(u) * cos(v)
    // y = r * cos(u) * sin(v)
    // z = r * sin(u)
    // where u: stack(latitude) angle (-90 <= u <= 90)
    //       v: sector(longitude) angle (0 <= v <= 360)
    //-----------------------------------------------------------------------------
    static void mvBuildVerticesSmooth(mvSphere& sphere, int sectorCount, int stackCount)
    {
        const float PI = acos(-1);

        float x, y, z, xy;                                  // vertex position
        float nx, ny, nz, lengthInv = 1.0f / sphere.radius; // normal
        float s, t;                                         // texCoord

        float sectorStep = 2 * PI / sectorCount;
        float stackStep = PI / stackCount;
        float sectorAngle, stackAngle;

        for (int i = 0; i <= stackCount; ++i)
        {
            stackAngle = PI / 2 - i * stackStep;   // starting from pi/2 to -pi/2
            xy = sphere.radius * cosf(stackAngle); // r * cos(u)
            z = sphere.radius * sinf(stackAngle);  // r * sin(u)

            // add (sectorCount+1) vertices per stack
            // the first and last vertices have same position and normal, but different tex coords
            for (int j = 0; j <= sectorCount; ++j)
            {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                // vertex position
                x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
                sphere.vertices.push_back(x);
                sphere.vertices.push_back(y);
                sphere.vertices.push_back(z);

                // normalized vertex normal
                nx = x * lengthInv;
                ny = y * lengthInv;
                nz = z * lengthInv;
                sphere.normals.push_back(nx);
                sphere.normals.push_back(ny);
                sphere.normals.push_back(nz);

                // vertex tex coord between [0, 1]
                s = (float)j / sectorCount;
                t = (float)i / stackCount;
                sphere.texCoords.push_back(s);
                sphere.texCoords.push_back(t);
            }
        }

        // indices
        //  k1--k1+1
        //  |  / |
        //  | /  |
        //  k2--k2+1
        unsigned int k1, k2;
        for (int i = 0; i < stackCount; ++i)
        {
            k1 = i * (sectorCount + 1);     // beginning of current stack
            k2 = k1 + sectorCount + 1;      // beginning of next stack

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                // 2 triangles per sector excluding 1st and last stacks
                if (i != 0)
                {
                    // k1---k2---k1+1
                    sphere.indices.push_back(k1);
                    sphere.indices.push_back(k2);
                    sphere.indices.push_back(k1 + 1); 
                }

                if (i != (stackCount - 1))
                {
                    // k1+1---k2---k2+1
                    sphere.indices.push_back(k1+1);
                    sphere.indices.push_back(k2);
                    sphere.indices.push_back(k2 + 1);
                }

                // vertical lines for all stacks
                sphere.lineIndices.push_back(k1);
                sphere.lineIndices.push_back(k2);
                if (i != 0)  // horizontal lines except 1st stack
                {
                    sphere.lineIndices.push_back(k1);
                    sphere.lineIndices.push_back(k1 + 1);
                }
            }
        }

        // generate interleaved vertex array as well
        mvBuildInterleavedVertices(sphere);
    }

    //-----------------------------------------------------------------------------
    // generate vertices with flat shading
    // each triangle is independent (no shared vertices)
    //-----------------------------------------------------------------------------
    static void mvBuildVerticesFlat(mvSphere& sphere, int sectorCount, int stackCount)
    {
        const float PI = acos(-1);

        // tmp vertex definition (x,y,z,s,t)
        struct Vertex
        {
            float x, y, z, s, t;
        };
        std::vector<Vertex> tmpVertices;

        float sectorStep = 2 * PI / sectorCount;
        float stackStep = PI / stackCount;
        float sectorAngle, stackAngle;

        // compute all vertices first, each vertex contains (x,y,z,s,t) except normal
        for (int i = 0; i <= stackCount; ++i)
        {
            stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            float xy = sphere.radius * cosf(stackAngle);       // r * cos(u)
            float z = sphere.radius * sinf(stackAngle);        // r * sin(u)

            // add (sectorCount+1) vertices per stack
            // the first and last vertices have same position and normal, but different tex coords
            for (int j = 0; j <= sectorCount; ++j)
            {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                Vertex vertex;
                vertex.x = xy * cosf(sectorAngle);      // x = r * cos(u) * cos(v)
                vertex.y = xy * sinf(sectorAngle);      // y = r * cos(u) * sin(v)
                vertex.z = z;                           // z = r * sin(u)
                vertex.s = (float)j / sectorCount;        // s
                vertex.t = (float)i / stackCount;         // t
                tmpVertices.push_back(vertex);
            }
        }

        Vertex v1, v2, v3, v4;                          // 4 vertex positions and tex coords
        std::vector<float> n;                           // 1 face normal

        int i, j, k, vi1, vi2;
        int index = 0;                                  // index for vertex
        for (i = 0; i < stackCount; ++i)
        {
            vi1 = i * (sectorCount + 1);                // index of tmpVertices
            vi2 = (i + 1) * (sectorCount + 1);

            for (j = 0; j < sectorCount; ++j, ++vi1, ++vi2)
            {
                // get 4 vertices per sector
                //  v1--v3
                //  |    |
                //  v2--v4
                v1 = tmpVertices[vi1];
                v2 = tmpVertices[vi2];
                v3 = tmpVertices[vi1 + 1];
                v4 = tmpVertices[vi2 + 1];

                // if 1st stack and last stack, store only 1 triangle per sector
                // otherwise, store 2 triangles (quad) per sector
                if (i == 0) // a triangle for first stack ==========================
                {
                    // put a triangle
                    sphere.vertices.push_back(v1.x);
                    sphere.vertices.push_back(v1.y);
                    sphere.vertices.push_back(v1.z);

                    sphere.vertices.push_back(v2.x);
                    sphere.vertices.push_back(v2.y);
                    sphere.vertices.push_back(v2.z);

                    sphere.vertices.push_back(v4.x);
                    sphere.vertices.push_back(v4.y);
                    sphere.vertices.push_back(v4.z);

                    // put tex coords of triangle
                    sphere.texCoords.push_back(v1.s);
                    sphere.texCoords.push_back(v1.t);

                    sphere.texCoords.push_back(v2.s);
                    sphere.texCoords.push_back(v2.t);

                    sphere.texCoords.push_back(v4.s);
                    sphere.texCoords.push_back(v4.t);

                    // put normal
                    n = mvComputeFaceNormal(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v4.x, v4.y, v4.z);
                    for (k = 0; k < 3; ++k)  // same normals for 3 vertices
                    {
                        sphere.normals.push_back(n[0]);
                        sphere.normals.push_back(n[1]);
                        sphere.normals.push_back(n[2]);
                    }

                    // put indices of 1 triangle
                    sphere.indices.push_back(index);
                    sphere.indices.push_back(index+1);
                    sphere.indices.push_back(index+2);

                    // indices for line (first stack requires only vertical line)
                    sphere.lineIndices.push_back(index);
                    sphere.lineIndices.push_back(index + 1);

                    index += 3;     // for next
                }
                else if (i == (stackCount - 1)) // a triangle for last stack =========
                {
                    // put a triangle
                    sphere.vertices.push_back(v1.x);
                    sphere.vertices.push_back(v1.y);
                    sphere.vertices.push_back(v1.z);

                    sphere.vertices.push_back(v2.x);
                    sphere.vertices.push_back(v2.y);
                    sphere.vertices.push_back(v2.z);

                    sphere.vertices.push_back(v3.x);
                    sphere.vertices.push_back(v3.y);
                    sphere.vertices.push_back(v3.z);

                    // put tex coords of triangle
                    sphere.texCoords.push_back(v1.s);
                    sphere.texCoords.push_back(v1.t);

                    sphere.texCoords.push_back(v2.s);
                    sphere.texCoords.push_back(v2.t);

                    sphere.texCoords.push_back(v3.s);
                    sphere.texCoords.push_back(v3.t);

                    // put normal
                    n = mvComputeFaceNormal(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v3.x, v3.y, v3.z);
                    for (k = 0; k < 3; ++k)  // same normals for 3 vertices
                    {
                        sphere.normals.push_back(n[0]);
                        sphere.normals.push_back(n[1]);
                        sphere.normals.push_back(n[2]);
                    }

                    // put indices of 1 triangle
                    sphere.indices.push_back(index);
                    sphere.indices.push_back(index + 1);
                    sphere.indices.push_back(index + 2);

                    // indices for lines (last stack requires both vert/hori lines)
                    sphere.lineIndices.push_back(index);
                    sphere.lineIndices.push_back(index + 1);
                    sphere.lineIndices.push_back(index);
                    sphere.lineIndices.push_back(index + 2);

                    index += 3;     // for next
                }
                else // 2 triangles for others ====================================
                {
                    // put quad vertices: v1-v2-v3-v4
                    sphere.vertices.push_back(v1.x);
                    sphere.vertices.push_back(v1.y);
                    sphere.vertices.push_back(v1.z);

                    sphere.vertices.push_back(v2.x);
                    sphere.vertices.push_back(v2.y);
                    sphere.vertices.push_back(v2.z);

                    sphere.vertices.push_back(v3.x);
                    sphere.vertices.push_back(v3.y);
                    sphere.vertices.push_back(v3.z);

                    sphere.vertices.push_back(v4.x);
                    sphere.vertices.push_back(v4.y);
                    sphere.vertices.push_back(v4.z);

                    // put tex coords of quad
                    sphere.texCoords.push_back(v1.s);
                    sphere.texCoords.push_back(v1.t);

                    sphere.texCoords.push_back(v2.s);
                    sphere.texCoords.push_back(v2.t);

                    sphere.texCoords.push_back(v3.s);
                    sphere.texCoords.push_back(v3.t);

                    sphere.texCoords.push_back(v4.s);
                    sphere.texCoords.push_back(v4.t);

                    // put normal
                    n = mvComputeFaceNormal(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v3.x, v3.y, v3.z);
                    for (k = 0; k < 4; ++k)  // same normals for 4 vertices
                    {
                        sphere.normals.push_back(n[0]);
                        sphere.normals.push_back(n[1]);
                        sphere.normals.push_back(n[2]);
                    }

                    // put indices of quad (2 triangles)
                    sphere.indices.push_back(index);
                    sphere.indices.push_back(index + 1);
                    sphere.indices.push_back(index + 2);

                    sphere.indices.push_back(index + 2);
                    sphere.indices.push_back(index + 1);
                    sphere.indices.push_back(index + 3);

                    // indices for lines
                    sphere.lineIndices.push_back(index);
                    sphere.lineIndices.push_back(index + 1);
                    sphere.lineIndices.push_back(index);
                    sphere.lineIndices.push_back(index + 2);

                    index += 4;     // for next
                }
            }
        }

        // generate interleaved vertex array as well
        mvBuildInterleavedVertices(sphere);
    }


    //-----------------------------------------------------------------------------
    // generate interleaved vertices: V/N/T
    // stride must be 32 bytes
    //-----------------------------------------------------------------------------
    static void mvBuildInterleavedVertices(mvSphere& sphere)
    {

        std::size_t i, j;
        std::size_t count = sphere.vertices.size();
        for (i = 0, j = 0; i < count; i += 3, j += 2)
        {
            sphere.interleavedVertices.push_back(sphere.vertices[i]);
            sphere.interleavedVertices.push_back(sphere.vertices[i + 1]);
            sphere.interleavedVertices.push_back(sphere.vertices[i + 2]);

            sphere.interleavedVertices.push_back(sphere.normals[i]);
            sphere.interleavedVertices.push_back(sphere.normals[i + 1]);
            sphere.interleavedVertices.push_back(sphere.normals[i + 2]);

            sphere.interleavedVertices.push_back(sphere.texCoords[j]);
            sphere.interleavedVertices.push_back(sphere.texCoords[j + 1]);
        }
    }


    //-----------------------------------------------------------------------------
    // return face normal of a triangle v1-v2-v3
    // if a triangle has no surface (normal length = 0), then return a zero vector
    //-----------------------------------------------------------------------------
    static std::vector<float> mvComputeFaceNormal(float x1, float y1, float z1,  // v1
        float x2, float y2, float z2,  // v2
        float x3, float y3, float z3)  // v3
    {
        const float EPSILON = 0.000001f;

        std::vector<float> normal(3, 0.0f);     // default return value (0,0,0)
        float nx, ny, nz;

        // find 2 edge vectors: v1-v2, v1-v3
        float ex1 = x2 - x1;
        float ey1 = y2 - y1;
        float ez1 = z2 - z1;
        float ex2 = x3 - x1;
        float ey2 = y3 - y1;
        float ez2 = z3 - z1;

        // cross product: e1 x e2
        nx = ey1 * ez2 - ez1 * ey2;
        ny = ez1 * ex2 - ex1 * ez2;
        nz = ex1 * ey2 - ey1 * ex2;

        // normalize only if the length is > 0
        float length = sqrtf(nx * nx + ny * ny + nz * nz);
        if (length > EPSILON)
        {
            // normalize
            float lengthInv = 1.0f / length;
            normal[0] = nx * lengthInv;
            normal[1] = ny * lengthInv;
            normal[2] = nz * lengthInv;
        }

        return normal;
    }

}