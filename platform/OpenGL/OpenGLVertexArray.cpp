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
    vertexBuffer->Bind();

    const auto &layout = vertexBuffer->GetLayout();
    unsigned int index = 0;

    for (const auto& element : layout)
    {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, element.GetComponentCount(), GL_FLOAT, GL_FALSE, layout.GetStride(), (const void*)(intptr_t)element.Offset);
        index++;
    }
    m_VertexBuffers.push_back(vertexBuffer);
};

void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer)
{
    indexBuffer->Bind();
    m_IndexBuffer = indexBuffer;
};
