#pragma once

#include "../pch.h"

#include "MathTools.h"
#include "Preprocessor.h"
#include "BufferLayout.h"
#include "Buffer.h"

namespace Engine
{

    class Mesh
    {
        public:
        Mesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices, const BufferLayout &bufferLayout, const TextureLayout &textureLayout) : m_Vertices(vertices), m_Indices(indices), m_BufferLayout(bufferLayout), m_TextureLayout(textureLayout) {}
        Mesh() {}

        std::vector<float> &GetVertices() { return m_Vertices; }
        std::vector<unsigned int> &GetIndices() { return m_Indices; }

        BufferLayout &GetLayout() { return m_BufferLayout; }
        void SetLayout(BufferLayout &layout) { m_BufferLayout = layout; }
        TextureLayout &GetTextureLayout() { return m_TextureLayout; }
        void SetTextureLayout(TextureLayout &layout) { m_TextureLayout = layout; }

        Shared<VertexArray> &GetVAO() { return m_VAO; }

        const AABB& GetAABB() const { return m_AABB; }
        void SetAABB(const AABB &aabb) { m_AABB = aabb; }

        void Prepare()
        {

            m_VAO = VertexArray::Create();
            m_VAO->Bind();
            m_VBO = VertexBuffer::Create(&m_Vertices[0], m_Vertices.size() * sizeof(float));
            m_EBO = IndexBuffer::Create(&m_Indices[0], m_Indices.size());
            m_VBO->SetLayout(m_BufferLayout);
            m_VAO->AddVertexBuffer(m_VBO);

            // Temp hack
            BufferLayout instanceLayout = {
            { ShaderDataType::Mat4, "a_InstanceTransform" },
            };

            auto instanceVBO = VertexBuffer::Create(1000 * sizeof(glm::mat4));
            instanceVBO->SetLayout(instanceLayout);
            m_VAO->AddVertexBuffer(instanceVBO);

            m_VAO->SetIndexBuffer(m_EBO);
            m_VAO->Unbind();
        }

        bool IsSameBufferLayout(BufferLayout &otherLayout)
        {
            return m_BufferLayout == otherLayout;
        }

        private:
        std::vector<float> m_Vertices;
        std::vector<unsigned int> m_Indices;
        Shared<VertexArray> m_VAO;
        Shared<VertexBuffer> m_VBO;
        Shared<IndexBuffer> m_EBO;
        BufferLayout m_BufferLayout;
        TextureLayout m_TextureLayout;
        AABB m_AABB;

        friend class Renderer;
    };

};
