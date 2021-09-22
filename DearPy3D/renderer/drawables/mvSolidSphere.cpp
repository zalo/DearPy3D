#include "mvSolidSphere.h"
#include "mvTexture.h"
#include "mvSampler.h"
#include "mvSphere.h"
#include <chrono>

namespace DearPy3D {

    mvDrawable mvCreateSolidSphere()
    {

        std::vector<float> vertices;
        std::vector<uint16_t> indices;
        std::vector<float> normals;
        mvSphere sphere = mvCreateSphere(0.25f);

        vertices = sphere.vertices;
        indices = sphere.indices;
        normals = sphere.normals;

        std::vector<float> nverticies;
        for (int i = 0; i < vertices.size(); i = i + 3)
        {
            nverticies.push_back(vertices[i]);
            nverticies.push_back(vertices[i + 1]);
            nverticies.push_back(vertices[i + 2]);
            nverticies.push_back(normals[i]);
            nverticies.push_back(normals[i + 1]);
            nverticies.push_back(normals[i + 2]);

            nverticies.push_back(0.0f);
            nverticies.push_back(0.0f);
            nverticies.push_back(0.0f);
            nverticies.push_back(0.0f);
            nverticies.push_back(0.0f);
            nverticies.push_back(0.0f);
            nverticies.push_back(0.0f);
            nverticies.push_back(0.0f);
        }

        mvDrawable drawable{};
        drawable.vertexBuffer = mvCreateBuffer(vertices.data(), vertices.size(), sizeof(float), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        drawable.indexBuffer = mvCreateBuffer(indices.data(), indices.size(), sizeof(uint16_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        return drawable;
    }

}
