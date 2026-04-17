#include "OpenGLBuffer.h"
#include "../../pch.h"

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
						(const void*)element.Offset);
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
						(const void*)element.Offset);
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

    static bool IsDepthFormat(FramebufferTextureFormat format)
    {
        switch (format)
        {
            case FramebufferTextureFormat::DEPTH24STENCIL8: return true;
        }

        return false;
    };

    static GLenum BufferTextureTypeToGL(BufferTextureType type)
    {
        switch (type)
        {
            case BufferTextureType::Texture2D: return GL_TEXTURE_2D;
            case BufferTextureType::Texture2D_Multisample: return GL_TEXTURE_2D_MULTISAMPLE;
            case BufferTextureType::Cubemap: return GL_TEXTURE_CUBE_MAP;
        }
    }

    static void CreateTexture(BufferTextureType type, unsigned int* outID, unsigned int count)
    {
        glCreateTextures(BufferTextureTypeToGL(type), count, outID);
    };

    static void AttachColorTexture(unsigned int id, BufferTextureType type, unsigned int samples, GLenum internalFormat, GLenum format, unsigned int width, unsigned int height, int index)
    {
        switch (type)
        {
            case BufferTextureType::Texture2D: {
                glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                break;
            }
            case BufferTextureType::Texture2D_Multisample: {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
                break;
            }
            case BufferTextureType::Cubemap: {
                for (unsigned int i = 0; i < 6; ++i)
                {
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
                }
                
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                break;
            }
        }

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, id, 0);
    };

    static void AttachDepthTexture(unsigned int id, BufferTextureType type, int samples, GLenum format, GLenum attachmentType, int width, unsigned int height)
    {
        switch (type)
        {
            case BufferTextureType::Texture2D: {
                glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                break;
            }
            case BufferTextureType::Texture2D_Multisample: {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
                break;
            }
            case BufferTextureType::Cubemap: {
                for (unsigned int i = 0; i < 6; ++i)
                {
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
                }
                
                // Note: Shadows usually look better with GL_NEAREST or hardware PCF, but GL_LINEAR is fine for testing
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                break;
            }
        }

        glFramebufferTexture(GL_FRAMEBUFFER, attachmentType, id, 0);
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

        if (m_ColorAttachmentSpecs.size())
        {
            m_ColorAttachments.resize(m_ColorAttachmentSpecs.size());
            glCreateTextures(BufferTextureTypeToGL(m_Specification.type), m_ColorAttachments.size(), m_ColorAttachments.data());
            for (unsigned int i = 0; i < m_ColorAttachments.size(); i++)
            {
                glBindTexture(BufferTextureTypeToGL(m_Specification.type), m_ColorAttachments[i]);
                switch (m_ColorAttachmentSpecs[i].TextureFormat)
                {
                    case FramebufferTextureFormat::RGBA8:
                        AttachColorTexture(m_ColorAttachments[i], m_Specification.type, m_Specification.samples, GL_RGBA8, GL_RGBA, m_Specification.width, m_Specification.height, i);
                        break;
                    case FramebufferTextureFormat::RED_INTEGER:
                        AttachColorTexture(m_ColorAttachments[i], m_Specification.type, m_Specification.samples, GL_R32I, GL_RED_INTEGER, m_Specification.width, m_Specification.height, i);
                        break;
                }
            };
        };

        if (m_DepthSpec.TextureFormat != FramebufferTextureFormat::None)
        {
            glCreateTextures(BufferTextureTypeToGL(m_Specification.type), 1, &m_DepthAttachment);
            glBindTexture(BufferTextureTypeToGL(m_Specification.type), m_DepthAttachment);
            switch (m_DepthSpec.TextureFormat)
            {
                case FramebufferTextureFormat::DEPTH24STENCIL8:
                    AttachDepthTexture(m_DepthAttachment, m_Specification.type, m_Specification.samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.width, m_Specification.height);
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
        // auto& spec = m_ColorAttachmentSpecs[attachmentIdx];
        // glClearTexImage(m_ColorAttachments[attachmentIdx], 0, FBTextureFormatToGL(spec.TextureFormat), GL_INT, &value);

        auto& spec = m_ColorAttachmentSpecs[attachmentIdx];

        // If this is an integer attachment (RED_INTEGER / R32I), clear via the bound framebuffer with glClearBufferiv.
        // This works regardless of multisampling on the FBO (glClearTexImage cannot be used on multisample textures).
        if (spec.TextureFormat == FramebufferTextureFormat::RED_INTEGER)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
            GLint clearValue = value;
            glClearBufferiv(GL_COLOR, attachmentIdx, &clearValue);
        }
        else
        {
            // For color attachments like RGBA8, clearing via glClearBufferfv is safe and simple.
            glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
            GLfloat clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
            // If you want to accept a color parameter, extend the API. For now we clear to zero.
            glClearBufferfv(GL_COLOR, attachmentIdx, clearColor);
        }
    };

};
