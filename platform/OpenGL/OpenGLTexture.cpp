#include "OpenGL/OpenGLTexture.h"
#include "pch.h"
#include "ktx.h"

namespace Engine
{

    OpenGLTexture2D::OpenGLTexture2D(const TextureLoadData &data)
    {
        m_Width = data.Width;
        m_Height = data.Height;

        if (data.HDRPixels)
        {
            m_InternalFormat = (data.Channels == 4) ? GL_RGBA16F : GL_RGB16F;
            m_DataFormat = (data.Channels == 4) ? GL_RGBA : GL_RGB;

            glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
            glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

            unsigned int clamped = data.clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
            glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, clamped);
            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, clamped);

            glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_FLOAT, data.HDRPixels);
            m_Loaded = true;
            return;
        }

        m_InternalFormat = GL_RGBA8;
        m_DataFormat = GL_RGBA;

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

        unsigned int clamped = data.clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, clamped);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, clamped);

        if (data.Pixels)
        {
            glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data.Pixels);
        }
    }

    void OpenGLTexture2D::Copy(uint32_t rendererID, uint32_t width, uint32_t height)
    {
        glCopyImageSubData(
            rendererID, GL_TEXTURE_2D, 0, 0, 0, 0,
            m_RendererID, GL_TEXTURE_2D, 0, 0, 0, 0,
            width, height, 1
        );
    }

    void OpenGLTexture2D::ResizeCopy(uint32_t srcRendererID, uint32_t srcWidth, uint32_t srcHeight)
    {
        GLuint readFBO, drawFBO;
        glCreateFramebuffers(1, &readFBO);
        glCreateFramebuffers(1, &drawFBO);

        glNamedFramebufferTexture(readFBO, GL_COLOR_ATTACHMENT0, srcRendererID, 0);
        glNamedFramebufferTexture(drawFBO, GL_COLOR_ATTACHMENT0, m_RendererID, 0);

        glBlitNamedFramebuffer(
            readFBO, drawFBO,
            0, 0, srcWidth, srcHeight,
            0, 0, m_Width, m_Height,
            GL_COLOR_BUFFER_BIT,
            GL_LINEAR
        );

        glDeleteFramebuffers(1, &readFBO);
        glDeleteFramebuffers(1, &drawFBO);
    }

    void OpenGLTexture2D::ReadPixels(std::vector<unsigned char>& out) const
    {
        out.resize((size_t)m_Width * m_Height * 4);
        glGetTextureImage(m_RendererID, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLsizei)out.size(), out.data());
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        glDeleteTextures(1, &m_RendererID);
    };

    void OpenGLTexture2D::Bind(unsigned int slot)
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
    };

    unsigned int OpenGLTexture2D::GetID() const
    {
        return m_RendererID;
    };

    OpenGLBindlessTexture::OpenGLBindlessTexture()
    {
    };

    void OpenGLBindlessTexture::Bind() const
    {
        glMakeTextureHandleResidentARB(m_RendererID);
    };

    void OpenGLBindlessTexture::Unbind() const
    {
        glMakeTextureHandleNonResidentARB(m_RendererID);
    };

    void OpenGLBindlessTexture::MakeHandle(const std::shared_ptr<OpenGLTexture2D> &texture)
    {
        m_RendererID = glGetTextureHandleARB(texture->GetID());
    };

};
