#include "OpenGLBuffer.h"
#include <glad/glad.h>

OpenGLVertexBuffer::OpenGLVertexBuffer(unsigned int size)
{
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
};

OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, unsigned int size)
{
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
};

OpenGLVertexBuffer::~OpenGLVertexBuffer()
{
    glDeleteBuffers(1, &m_RendererID);
};

void OpenGLVertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
};

void OpenGLVertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
};

OpenGLIndexBuffer::OpenGLIndexBuffer(unsigned int* indices, unsigned int count) : m_Count(count)
{
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
};

OpenGLIndexBuffer::~OpenGLIndexBuffer()
{
    glDeleteBuffers(1, &m_RendererID);
};

void OpenGLIndexBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
};

void OpenGLIndexBuffer::Unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
};

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
        GLenum type = element.Type == ShaderDataType::Int ? GL_INT : GL_FLOAT;

        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, element.GetComponentCount(), type, GL_FALSE, layout.GetStride(), (const void*)(intptr_t)element.Offset);
        index++;
    }
    m_VertexBuffers.push_back(vertexBuffer);
};

void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer)
{
    indexBuffer->Bind();
    m_IndexBuffer = indexBuffer;
};

OpenGLSSBOBuffer::OpenGLSSBOBuffer(void* data, unsigned int size)
{
    glCreateBuffers(1, &m_RendererID);
    glNamedBufferStorage(m_RendererID, size, data, GL_DYNAMIC_STORAGE_BIT);
};

OpenGLSSBOBuffer::~OpenGLSSBOBuffer()
{
};

void OpenGLSSBOBuffer::Bind(unsigned int index) const
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_RendererID);
};

void OpenGLSSBOBuffer::Unbind() const
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
};

