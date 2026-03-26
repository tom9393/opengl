#pragma once

#include "../pch.h"

#include "Preprocessor.h"
#include "BufferLayout.h"
#include "Buffer.h"

namespace Engine
{

    class Mesh
    {
        public:
        Mesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices, const BufferLayout &bufferLayout, const TextureLayout &textureLayout) : m_Vertices(vertices), m_Indices(indices), m_BufferLayout(bufferLayout), m_TextureLayout(textureLayout) {}

        std::vector<float> &GetVertices() { return m_Vertices; }
        std::vector<unsigned int> &GetIndices() { return m_Indices; }
        BufferLayout &GetLayout() { return m_BufferLayout; }
        Shared<VertexArray> &GetVAO() { return m_VAO; }
        TextureLayout &GetTextureLayout() { return m_TextureLayout; }

        void Prepare()
        {
            m_VAO = VertexArray::Create();
            m_VAO->Bind();
            m_VBO = VertexBuffer::Create(&m_Vertices[0], m_Vertices.size() * sizeof(float));
            m_EBO = IndexBuffer::Create(&m_Indices[0], m_Indices.size());
            m_VBO->SetLayout(m_BufferLayout);
            m_VAO->AddVertexBuffer(m_VBO);
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

        friend class Renderer;
    };

};
