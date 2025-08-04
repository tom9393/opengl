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
    Mesh(std::vector<float> &vertices, std::vector<unsigned int> &indices, std::vector<std::shared_ptr<Texture2D>> &textures, BufferLayout &layout) : m_Vertices(vertices), m_Indices(indices), m_Textures(textures), m_Layout(layout)
    {
        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    std::shared_ptr<VertexArray> GetVao() { return VAO; }
    std::vector<std::shared_ptr<Texture2D>> GetTextures() { return m_Textures; }

private:
    // render data 
    std::shared_ptr<VertexArray> VAO;
    std::shared_ptr<VertexBuffer> VBO;
    std::shared_ptr<IndexBuffer> EBO;

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        // create buffers/arrays
        VAO = VertexArray::Create();
        VBO = VertexBuffer::Create(m_Vertices.data(), m_Vertices.size() * sizeof(float));
        EBO = IndexBuffer::Create(m_Indices.data(), m_Indices.size() * sizeof(unsigned int));
        VBO->SetLayout(m_Layout);
        VAO->AddVertexBuffer(VBO);
        VAO->SetIndexBuffer(EBO);
    }
};

