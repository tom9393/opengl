#include "pch.h"
#include "OpenGLBuffer.h"

namespace Engine
{

    static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:    return GL_FLOAT;
			case ShaderDataType::Float2:   return GL_FLOAT;
			case ShaderDataType::Float3:   return GL_FLOAT;
			case ShaderDataType::Float4:   return GL_FLOAT;
			case ShaderDataType::Mat3:     return GL_FLOAT;
			case ShaderDataType::Mat4:     return GL_FLOAT;
			case ShaderDataType::Int:      return GL_INT;
			case ShaderDataType::Int2:     return GL_INT;
			case ShaderDataType::Int3:     return GL_INT;
			case ShaderDataType::Int4:     return GL_INT;
			case ShaderDataType::Bool:     return GL_BOOL;
		}

		return 0;
	}

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

    void OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void OpenGLVertexBuffer::EnableAttribute(uint32_t index)
    {
		glEnableVertexAttribArray(index);
    }

    void OpenGLVertexBuffer::DisableAttribute(uint32_t index)
    {
		glDisableVertexAttribArray(index);
    }

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
        glDeleteVertexArrays(1, &m_RendererID);
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
        Bind();
        vertexBuffer->Bind();

        const auto &layout = vertexBuffer->GetLayout();

        for (const auto& element : layout)
		{
			switch (element.Type)
			{
				case ShaderDataType::Float:
				case ShaderDataType::Float2:
				case ShaderDataType::Float3:
				case ShaderDataType::Float4:
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribPointer(m_VertexBufferIndex,
						element.GetComponentCount(),
						ShaderDataTypeToOpenGLBaseType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)(intptr_t)element.Offset);
					m_VertexBufferIndex++;
					break;
				}
				case ShaderDataType::Int:
				case ShaderDataType::Int2:
				case ShaderDataType::Int3:
				case ShaderDataType::Int4:
				case ShaderDataType::Bool:
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribIPointer(m_VertexBufferIndex,
						element.GetComponentCount(),
						ShaderDataTypeToOpenGLBaseType(element.Type),
						layout.GetStride(),
						(const void*)(intptr_t)element.Offset);
					m_VertexBufferIndex++;
					break;
				}
				case ShaderDataType::Mat3:
				case ShaderDataType::Mat4:
				{
                    // A mat4 has 4 rows (vec4s), a mat3 has 3 rows (vec3s)
                    uint8_t rowCount = (element.Type == ShaderDataType::Mat4) ? 4 : 3;

                    for (uint8_t i = 0; i < rowCount; i++)
                    {
                        glEnableVertexAttribArray(m_VertexBufferIndex);
                        glVertexAttribPointer(m_VertexBufferIndex,
                            rowCount, // 4 floats per row
                            ShaderDataTypeToOpenGLBaseType(element.Type),
                            element.Normalized ? GL_TRUE : GL_FALSE,
                            layout.GetStride(), // This stride will now just be sizeof(mat4)
                            (const void*)(element.Offset + sizeof(float) * rowCount * i)); // Offset by 16 bytes per row

                        // This makes it per-instance!
                        glVertexAttribDivisor(m_VertexBufferIndex, 1);
                        m_VertexBufferIndex++;
                    }
                    break;
				}
			}
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
        glDeleteBuffers(1, &m_RendererID);
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

    OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(unsigned int size)
    {
        glCreateBuffers(1, &m_RendererID);
        glNamedBufferStorage(m_RendererID, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
    };

    OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(void* data, unsigned int size)
    {
        glCreateBuffers(1, &m_RendererID);
        glNamedBufferStorage(m_RendererID, size, data, GL_DYNAMIC_STORAGE_BIT);
    };

    OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer()
    {
        glDeleteBuffers(1, &m_RendererID);
    };

    void OpenGLShaderStorageBuffer::Bind(unsigned int index) const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_RendererID);
    };

    void OpenGLShaderStorageBuffer::Unbind() const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    };

    void OpenGLShaderStorageBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
    }

    GLenum BufferTextureTypeToGL(BufferTextureType type)
    {
        switch (type)
        {
            case BufferTextureType::Texture2D: return GL_TEXTURE_2D;
            case BufferTextureType::Texture2D_Array: return GL_TEXTURE_2D_ARRAY;
            case BufferTextureType::Texture2D_Multisample: return GL_TEXTURE_2D_MULTISAMPLE;
            case BufferTextureType::Cubemap: return GL_TEXTURE_CUBE_MAP;
            case BufferTextureType::Cubemap_Array: return GL_TEXTURE_CUBE_MAP_ARRAY;
            default: return GL_NONE;
        }
    }

    struct GLFormat { GLenum internal; GLenum pixel; GLenum data; };

    static GLFormat ToGLFormat(FramebufferTextureFormat format)
    {
        switch (format)
        {
            case FramebufferTextureFormat::RGBA8:           return { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE };
            case FramebufferTextureFormat::RGBA16F:         return { GL_RGBA16F, GL_RGBA, GL_FLOAT };
            case FramebufferTextureFormat::RED_INTEGER:     return { GL_R32I, GL_RED_INTEGER, GL_INT };
            case FramebufferTextureFormat::DEPTH24STENCIL8: return { GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 };
            case FramebufferTextureFormat::Depth:           return { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT };
            default:                                        return { GL_NONE, GL_NONE, GL_NONE };
        }
    }

    static GLenum DepthAttachmentPoint(FramebufferTextureFormat format)
    {
        return format == FramebufferTextureFormat::DEPTH24STENCIL8 ? GL_DEPTH_STENCIL_ATTACHMENT
                                                                   : GL_DEPTH_ATTACHMENT;
    }

    OpenGLRenderTarget::OpenGLRenderTarget(const RenderTargetSpecification& specification)
        : m_Specification(specification)
    {
        glCreateTextures(BufferTextureTypeToGL(m_Specification.type), 1, &m_RendererID);
        AllocateStorage();
        SetSamplerParameters();
    }

    OpenGLRenderTarget::~OpenGLRenderTarget()
    {
        glDeleteTextures(1, &m_RendererID);
    }

    // Storage uses the mutable glTexImage* family so Resize can respecify it in
    // place without changing the renderer id.
    void OpenGLRenderTarget::AllocateStorage()
    {
        GLenum target = BufferTextureTypeToGL(m_Specification.type);
        GLFormat fmt = ToGLFormat(m_Specification.format);
        unsigned int w = m_Specification.width, h = m_Specification.height;

        glBindTexture(target, m_RendererID);
        switch (m_Specification.type)
        {
            case BufferTextureType::Texture2D:
                glTexImage2D(GL_TEXTURE_2D, 0, fmt.internal, w, h, 0, fmt.pixel, fmt.data, nullptr);
                break;
            case BufferTextureType::Texture2D_Array:
                glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, fmt.internal, w, h, m_Specification.layers, 0, fmt.pixel, fmt.data, nullptr);
                break;
            case BufferTextureType::Texture2D_Multisample:
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_Specification.samples, fmt.internal, w, h, GL_FALSE);
                break;
            case BufferTextureType::Cubemap:
                for (unsigned int i = 0; i < 6; ++i)
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, fmt.internal, w, h, 0, fmt.pixel, fmt.data, nullptr);
                break;
            case BufferTextureType::Cubemap_Array:
                glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, fmt.internal, w, h, m_Specification.layers * 6, 0, fmt.pixel, fmt.data, nullptr);
                break;
            default:
                break;
        }
    }

    void OpenGLRenderTarget::SetSamplerParameters()
    {
        // Sampler parameters are illegal on multisample textures.
        if (m_Specification.type == BufferTextureType::Texture2D_Multisample) return;

        GLenum target = BufferTextureTypeToGL(m_Specification.type);
        bool depth = IsDepthFormat(m_Specification.format);
        bool array = m_Specification.type == BufferTextureType::Texture2D_Array ||
                     m_Specification.type == BufferTextureType::Cubemap_Array;

        // Depth arrays (shadow maps) sample with NEAREST; everything else LINEAR.
        GLenum filter = (depth && array) ? GL_NEAREST : GL_LINEAR;
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);

        if (depth && m_Specification.type == BufferTextureType::Texture2D_Array)
        {
            // Outside a 2D shadow map means "not in shadow".
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, borderColor);
        }
        else
        {
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            if (target == GL_TEXTURE_CUBE_MAP || target == GL_TEXTURE_CUBE_MAP_ARRAY)
                glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        }
    }

    void OpenGLRenderTarget::Resize(unsigned int width, unsigned int height)
    {
        if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
        {
            std::cerr << "OpenGLRenderTarget: invalid size " << width << "x" << height << std::endl;
            return;
        }
        if (width == m_Specification.width && height == m_Specification.height) return;

        m_Specification.width = width;
        m_Specification.height = height;
        AllocateStorage();
    }

    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& specification)
    {
        glCreateFramebuffers(1, &m_RendererID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

        for (auto& target : specification.attachments)
        {
            if (!target) continue;
            if (IsDepthFormat(target->GetSpecification().format))
            {
                if (m_DepthAttachment)
                    std::cerr << "OpenGLFramebuffer: more than one depth attachment." << std::endl;
                m_DepthAttachment = target;
                glFramebufferTexture(GL_FRAMEBUFFER, DepthAttachmentPoint(target->GetSpecification().format), target->GetID(), 0);
            }
            else
            {
                glFramebufferTexture(GL_FRAMEBUFFER, (GLenum)(GL_COLOR_ATTACHMENT0 + m_ColorAttachments.size()), target->GetID(), 0);
                m_ColorAttachments.push_back(target);
            }
        }

        if (!m_ColorAttachments.empty())
        {
            std::vector<GLenum> buffers(m_ColorAttachments.size());
            for (size_t i = 0; i < buffers.size(); i++) buffers[i] = (GLenum)(GL_COLOR_ATTACHMENT0 + i);
            glDrawBuffers(buffers.size(), buffers.data());
        }
        else
        {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "OpenGLFramebuffer: incomplete framebuffer (status 0x"
                      << std::hex << status << std::dec << ")." << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        glDeleteFramebuffers(1, &m_RendererID);
    }

    void OpenGLFramebuffer::Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

        // Attachment storage is respecified in place on resize, so the attachments'
        // current size is always the framebuffer's size.
        const Shared<RenderTarget>& any = m_ColorAttachments.empty() ? m_DepthAttachment : m_ColorAttachments[0];
        if (any)
        {
            const RenderTargetSpecification& spec = any->GetSpecification();
            glViewport(0, 0, spec.width, spec.height);
        }
    };

    void OpenGLFramebuffer::Unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    };

    int OpenGLFramebuffer::ReadPixel(unsigned int attachmentIdx, int x, int y)
    {
        if (attachmentIdx >= m_ColorAttachments.size() ||
            m_ColorAttachments[attachmentIdx]->GetSpecification().format != FramebufferTextureFormat::RED_INTEGER)
        {
            std::cerr << "OpenGLFramebuffer::ReadPixel: attachment " << attachmentIdx
                      << " is not a RED_INTEGER attachment." << std::endl;
            return -1;
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RendererID);
        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIdx);

        int pixelData = -1;
        glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

        return pixelData;
    };

    void OpenGLFramebuffer::ClearAttachment(unsigned int attachmentIdx, int value)
    {
        if (attachmentIdx >= m_ColorAttachments.size()) return;

        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        if (m_ColorAttachments[attachmentIdx]->GetSpecification().format == FramebufferTextureFormat::RED_INTEGER)
        {
            GLint clearValue = value;
            glClearBufferiv(GL_COLOR, attachmentIdx, &clearValue);
        }
        else
        {
            GLfloat clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            glClearBufferfv(GL_COLOR, attachmentIdx, clearColor);
        }
    };

    void OpenGLFramebuffer::AttachLayer(unsigned int layer)
    {
        if (!m_DepthAttachment) return;

        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        GLenum attachment = DepthAttachmentPoint(m_DepthAttachment->GetSpecification().format);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, m_DepthAttachment->GetID(), 0, layer);
    }

};
