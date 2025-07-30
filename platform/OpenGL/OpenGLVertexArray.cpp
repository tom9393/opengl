#include "OpenGLVertexArray.h"

OpenGLVertexArray::OpenGLVertexArray()
{
    glGenVertexArrays(1, &m_RendererID);
}

OpenGLVertexArray::~OpenGLVertexArray()
{
};

void OpenGLVertexArray::Bind() const
{
    glBindVertexArray(m_RendererID);
};

void OpenGLVertexArray::Unbind() const
{
    glBindVertexArray(0);
};

void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer)
{
    glBindVertexArray(m_RendererID);
    vertexBuffer->Bind();

    const auto &layout = vertexBuffer->GetLayout();
    uint32_t index = 0;
    for (const auto &element : layout)
    {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, element.GetComponentCount(), GL_FLOAT, GL_FALSE, layout.GetStride(), (const void*) element.Offset);
        index++;
    }
    m_VertexBuffers.push_back(vertexBuffer);
};

void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer)
{
    glBindVertexArray(m_RendererID);
    indexBuffer->Bind();

    m_IndexBuffer = indexBuffer;
};
