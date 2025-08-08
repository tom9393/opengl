#pragma once

#include "../pch.h"

#include "../include/preprocessor.h"
#include "AABB.h"
#include "BufferLayout.h"
#include "Texture.h"

class Mesh {
public:
    // mesh Data
    std::vector<float> m_Vertices;
    std::vector<unsigned int> m_Indices;
    std::vector<std::shared_ptr<Texture2D>> m_Textures;
    BufferLayout m_Layout;
    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 position = {0.0f, 0.0f, 0.0f};

    // constructor
    Mesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices, const std::vector<std::shared_ptr<Texture2D>> &textures, const BufferLayout &layout, glm::vec3 min, glm::vec3 max) : m_Vertices(vertices), m_Indices(indices), m_Textures(textures), m_Layout(layout), min(min), max(max) {}

    std::vector<std::shared_ptr<Texture2D>> &GetTextures() { return m_Textures; }
    std::vector<float> &GetVertices() { return m_Vertices; }
    std::vector<unsigned int> &GetIndices() { return m_Indices; }
    BufferLayout &GetLayout() { return m_Layout; }

    AABB GetTransformedAABB()
    {
        // Convert all 8 corners of the local AABB to world space
        glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
        glm::vec3 corners[8];

        corners[0] = glm::vec3(model * glm::vec4(min.x, min.y, min.z, 1.0f));
        corners[1] = glm::vec3(model * glm::vec4(max.x, min.y, min.z, 1.0f));
        corners[2] = glm::vec3(model * glm::vec4(min.x, max.y, min.z, 1.0f));
        corners[3] = glm::vec3(model * glm::vec4(max.x, max.y, min.z, 1.0f));
        corners[4] = glm::vec3(model * glm::vec4(min.x, min.y, max.z, 1.0f));
        corners[5] = glm::vec3(model * glm::vec4(max.x, min.y, max.z, 1.0f));
        corners[6] = glm::vec3(model * glm::vec4(min.x, max.y, max.z, 1.0f));
        corners[7] = glm::vec3(model * glm::vec4(max.x, max.y, max.z, 1.0f));

        glm::vec3 transformedMin = corners[0];
        glm::vec3 transformedMax = corners[0];
        for (int i = 1; i < 8; i++)
        {
            transformedMin = glm::min(transformedMin, corners[i]);
            transformedMax = glm::max(transformedMax, corners[i]);
        }

        return { transformedMin, transformedMax };
    }
};

