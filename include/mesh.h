#pragma once

#include "../pch.h"

#include "../include/preprocessor.h"
#include "BufferLayout.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"

class Mesh {
public:
    // mesh Data
    std::vector<float> m_Vertices;
    std::vector<unsigned int> m_Indices;
    std::vector<std::shared_ptr<Texture2D>> m_Textures;
    BufferLayout m_Layout;

    // constructor
    Mesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices, const std::vector<std::shared_ptr<Texture2D>> &textures, const BufferLayout &layout) : m_Vertices(vertices), m_Indices(indices), m_Textures(textures), m_Layout(layout)
    {
        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    std::shared_ptr<VertexArray> GetVao() const { return m_VAO; }
    std::vector<std::shared_ptr<Texture2D>> GetTextures() const { return m_Textures; }

private:
    // render data 
    std::shared_ptr<VertexArray> m_VAO;
    std::shared_ptr<VertexBuffer> m_VBO;
    std::shared_ptr<IndexBuffer> m_EBO;

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        // create buffers/arrays
        m_VAO = VertexArray::Create();
        m_VAO->Bind();
        m_VBO = VertexBuffer::Create(&m_Vertices[0], m_Vertices.size() * sizeof(float));
        m_EBO = IndexBuffer::Create(&m_Indices[0], m_Indices.size() * sizeof(unsigned int));
        // std::cout << "Vertex count: " << m_Vertices.size() << "\n";
        // std::cout << "Float per vertex: " << m_Layout.GetStride() / sizeof(float) << "\n";
        // std::cout << "Expected vertex count: " << m_Vertices.size() / (m_Layout.GetStride() / sizeof(float)) << "\n";
        m_VBO->SetLayout(m_Layout);
        m_VAO->AddVertexBuffer(m_VBO);
        m_VAO->SetIndexBuffer(m_EBO);
        m_VAO->Unbind();
    }
};

