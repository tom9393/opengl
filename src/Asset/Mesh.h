#pragma once

#include "../pch.h"

#include "../Utils/MathTools.h"
#include "../Core/Preprocessor.h"
#include "../Render/BufferLayout.h"
#include "../Render/Buffer.h"

namespace Engine
{

    class Mesh : public Asset
    {
        public:
        Mesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices, const Shared<VertexArray> &VAO, const std::vector<Shared<VertexBuffer>> &VBOs, const Shared<IndexBuffer> &EBO) : m_Vertices(vertices), m_Indices(indices), m_VAO(VAO), m_VBOs(VBOs), m_EBO(EBO) {}
        Mesh() {}

        std::vector<float> &GetVertices() { return m_Vertices; }
        std::vector<unsigned int> &GetIndices() { return m_Indices; }

        Shared<VertexArray> &GetVAO() { return m_VAO; }
        void SetVAO(Shared<VertexArray> &VAO) { m_VAO = VAO; }

        std::vector<Shared<VertexBuffer>> &GetVBOs() { return m_VBOs; }
        void SetVBOs(std::vector<Shared<VertexBuffer>> &VBOs) { m_VBOs = VBOs; }

        Shared<IndexBuffer> &GetEBO() { return m_EBO; }
        void SetEBO(Shared<IndexBuffer> &EBO) { m_EBO = EBO; }

        const AABB& GetAABB() const { return m_AABB; }
        void SetAABB(const AABB &aabb) { m_AABB = aabb; }

        private:
        std::vector<float> m_Vertices;
        std::vector<unsigned int> m_Indices;
        Shared<VertexArray> m_VAO;
        std::vector<Shared<VertexBuffer>> m_VBOs;
        Shared<IndexBuffer> m_EBO;
        AABB m_AABB;
    };

};
