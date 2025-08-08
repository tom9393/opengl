#include "OpenGLBuffer.h"
#include <GL/gl.h>
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

OpenGLFramebuffer::OpenGLFramebuffer(FramebufferSpecification &specification) : m_Specification(specification)
{
    Resize();
};

OpenGLFramebuffer::~OpenGLFramebuffer()
{
    glDeleteFramebuffers(1, &m_RendererID);
};

void OpenGLFramebuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
};

void OpenGLFramebuffer::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
};

void OpenGLFramebuffer::Resize()
{
    glCreateFramebuffers(1, &m_RendererID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachment);
    glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Specification.width, m_Specification.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);
    glBindTexture(GL_TEXTURE_2D, m_DepthAttachment);
    //glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_Specification.width, m_Specification.height);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Specification.width, m_Specification.height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
};
