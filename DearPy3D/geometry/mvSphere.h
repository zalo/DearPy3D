#pragma once

#include <vector>

namespace DearPy3D {

    struct mvSphere
    {
        float                 radius;
        int                   interleavedStride;
        std::vector<uint16_t> indices;
        std::vector<uint16_t> lineIndices;
        std::vector<float>    vertices;
        std::vector<float>    normals;
        std::vector<float>    texCoords;
        std::vector<float>    interleavedVertices; // # of bytes to hop to the next vertex (should be 32 bytes)
        
    };

    mvSphere mvCreateSphere(float radius = 1.0f, int sectorCount = 36, int stackCount = 18, bool smooth = true);
}
