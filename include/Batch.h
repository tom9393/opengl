#pragma once

#include "Buffer.h"

class Batch
{
    public:

    Batch(BufferLayout &layout) : m_Layout(layout), m_Ready(false), m_VertexCount(0) {}

    bool IsSameBufferLayout(BufferLayout &otherLayout)
    {
        return m_Layout == otherLayout;
    }

    void AddVertices(std::vector<float> &vertices)
    {
        m_Vertices.insert(m_Vertices.end(), vertices.begin(), vertices.end());
        m_VertexCount += vertices.size() / (m_Layout.GetStride() / sizeof(float));
    }

    void AddIndices(std::vector<unsigned int> &indices)
    {
        m_Indices.reserve(m_Indices.size() + indices.size());
        for (unsigned int i : indices) m_Indices.emplace_back(i + m_VertexCount);
    }

    bool IsReady() { return m_Ready; }

    void Prepare()
    {
        m_VAO = VertexArray::Create();
        m_VAO->Bind();
        m_VBO = VertexBuffer::Create(&m_Vertices[0], m_Vertices.size() * sizeof(float));
        m_EBO = IndexBuffer::Create(&m_Indices[0], m_Indices.size());
        m_VBO->SetLayout(m_Layout);
        m_VAO->AddVertexBuffer(m_VBO);
        m_VAO->SetIndexBuffer(m_EBO);
        m_VAO->Unbind();
        m_Ready = true;
    }

    std::shared_ptr<VertexArray> &GetVAO() { return m_VAO; }
    BufferLayout &GetLayout() { return m_Layout; }

    private:
    std::shared_ptr<VertexArray> m_VAO;
    std::shared_ptr<VertexBuffer> m_VBO;
    std::shared_ptr<IndexBuffer> m_EBO;
    std::vector<float> m_Vertices;
    std::vector<unsigned int> m_Indices;
    unsigned int m_VertexCount;
    BufferLayout m_Layout;
    bool m_Ready;
};
