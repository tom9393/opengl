#include "OpenGLBuffer.h"
#include "../../pch.h"

namespace Engine
{

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
            glEnableVertexAttribArray(index);

            switch (element.Type)
            {
                case ShaderDataType::Int:
                case ShaderDataType::Int2:
                case ShaderDataType::Int3:
                case ShaderDataType::Int4:
                case ShaderDataType::Bool:
                    glVertexAttribIPointer(index, element.GetComponentCount(), GL_INT, layout.GetStride(), (const void*)(intptr_t)element.Offset);
                    break;
                case ShaderDataType::Float:
                case ShaderDataType::Float2:
                case ShaderDataType::Float3:
                case ShaderDataType::Float4:
                    glVertexAttribPointer(index, element.GetComponentCount(), GL_FLOAT, GL_FALSE, layout.GetStride(), (const void*)(intptr_t)element.Offset);
                    break;
            };
            index++;
        }
        m_VertexBuffers.push_back(vertexBuffer);
    };

    void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer)
    {
        indexBuffer->Bind();
        m_IndexBuffer = indexBuffer;
    };

    OpenGLUniformBuffer::OpenGLUniformBuffer(void *data, unsigned int size)
    {
        glCreateBuffers(1, &m_RendererID);
        glNamedBufferStorage(m_RendererID, size, data, GL_DYNAMIC_STORAGE_BIT);
    };

    OpenGLUniformBuffer::~OpenGLUniformBuffer()
    {
    };

    void OpenGLUniformBuffer::Bind(unsigned int index) const
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, index, m_RendererID);
    };

    void OpenGLUniformBuffer::Unbind() const
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
    };

    void OpenGLUniformBuffer::Update(unsigned int offset, unsigned int size, void* data)
    {
        glNamedBufferSubData(m_RendererID, offset, size, data);
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

    static bool IsDepthFormat(FramebufferTextureFormat format)
    {
        switch (format)
        {
            case FramebufferTextureFormat::DEPTH24STENCIL8: return true;
        }

        return false;
    };

    static GLenum TextureTarget(bool multisampled)
    {
        return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    };

    static void CreateTexture(bool multisampled, unsigned int* outID, unsigned int count)
    {
        glCreateTextures(TextureTarget(multisampled), count, outID);
    };

    static void BindTexture(bool multisampled, unsigned int id)
    {
        glBindTexture(TextureTarget(multisampled), id);
    };

    static void AttachColorTexture(unsigned int id, int samples, GLenum internalFormat, GLenum format, unsigned int width, unsigned int height, int index)
    {
        bool multisampled = samples > 1;
        if (multisampled)
        {
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        };

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
    };

    static void AttachDepthTexture(unsigned int id, int samples, GLenum format, GLenum attachmentType, int width, unsigned int height)
    {
        bool multisampled = samples > 1;
        if (multisampled)
        {
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
        }
        else
        {
            glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        };

        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
    };

    static GLenum FBTextureFormatToGL(FramebufferTextureFormat format)
    {
        switch (format)
        {
            case FramebufferTextureFormat::RGBA8: return GL_RGBA8;
            case FramebufferTextureFormat::RED_INTEGER: return GL_RED_INTEGER;
        };
        
        return 0;
    };

    OpenGLFramebuffer::OpenGLFramebuffer(FramebufferSpecification &specification) : m_Specification(specification)
    {

        for (auto format : m_Specification.attachments.Attachments)
        {
            if (!IsDepthFormat(format.TextureFormat)) m_ColorAttachmentSpecs.emplace_back(format);
            else m_DepthSpec = format;
        };

        Invalidate();
    };

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        glDeleteFramebuffers(1, &m_RendererID);
        glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
        glDeleteTextures(1, &m_DepthAttachment);
    };

    void OpenGLFramebuffer::Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        glViewport(0, 0, m_Specification.width, m_Specification.height);
    };

    void OpenGLFramebuffer::Unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    };

    void OpenGLFramebuffer::Invalidate()
    {
        if (m_RendererID)
        {
            glDeleteFramebuffers(1, &m_RendererID);
            glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
            glDeleteTextures(1, &m_DepthAttachment);

            m_ColorAttachments.clear();
            m_DepthAttachment = 0;
        }

        glCreateFramebuffers(1, &m_RendererID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

        bool multisample = m_Specification.samples > 1;

        if (m_ColorAttachmentSpecs.size())
        {
            m_ColorAttachments.resize(m_ColorAttachmentSpecs.size());
            CreateTexture(multisample, m_ColorAttachments.data(), m_ColorAttachments.size());

            for (unsigned int i = 0; i < m_ColorAttachments.size(); i++)
            {
                BindTexture(multisample, m_ColorAttachments[i]);
                switch (m_ColorAttachmentSpecs[i].TextureFormat)
                {
                    case FramebufferTextureFormat::RGBA8:
                        AttachColorTexture(m_ColorAttachments[i], m_Specification.samples, GL_RGBA8, GL_RGBA, m_Specification.width, m_Specification.height, i);
                        break;
                    case FramebufferTextureFormat::RED_INTEGER:
                        AttachColorTexture(m_ColorAttachments[i], m_Specification.samples, GL_R32I, GL_RED_INTEGER, m_Specification.width, m_Specification.height, i);
                        break;
                }
            };
        };

        if (m_DepthSpec.TextureFormat != FramebufferTextureFormat::None)
        {
            CreateTexture(multisample, &m_DepthAttachment, 1);
            BindTexture(multisample, m_DepthAttachment);
            switch (m_DepthSpec.TextureFormat)
            {
                case FramebufferTextureFormat::DEPTH24STENCIL8:
                    AttachDepthTexture(m_DepthAttachment, m_Specification.samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.width, m_Specification.height);
                    break;
            };
        };

        if (m_ColorAttachments.size() > 1)
        {
            GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
            glDrawBuffers(m_ColorAttachments.size(), buffers);
        }
        else if (m_ColorAttachments.empty())
        {
            glDrawBuffer(GL_NONE);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    };

    void OpenGLFramebuffer::Resize(unsigned int width, unsigned int height)
    {
        if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
        {
            std::cout << "Invalid size" << std::endl;
        }
        m_Specification.width = width;
        m_Specification.height = height;
        Invalidate();
    };

    int OpenGLFramebuffer::ReadPixel(unsigned int attachmentIdx, int x, int y)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RendererID);
        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIdx);

        int pixelData = -1;
        glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);

        return pixelData;
    };

    void OpenGLFramebuffer::ClearAttachment(unsigned int attachmentIdx, int value)
    {
        auto& spec = m_ColorAttachmentSpecs[attachmentIdx];
        glClearTexImage(m_ColorAttachments[attachmentIdx], 0, FBTextureFormatToGL(spec.TextureFormat), GL_INT, &value);
    };

};
