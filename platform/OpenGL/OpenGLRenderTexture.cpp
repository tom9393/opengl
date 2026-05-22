#include "pch.h"
#include "OpenGL/OpenGLRenderTexture.h"

namespace Engine
{

    static GLenum InternalFormat(RenderTextureFormat format)
    {
        switch (format)
        {
            case RenderTextureFormat::RGBA16F: return GL_RGBA16F;
            case RenderTextureFormat::RG16F:   return GL_RG16F;
        }
        return GL_RGBA16F;
    }

    OpenGLRenderTexture::OpenGLRenderTexture(const RenderTextureSpecification& spec) : m_Spec(spec)
    {
        m_Target = (spec.type == RenderTextureType::Cubemap) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;

        glGenTextures(1, &m_TextureID);
        glBindTexture(m_Target, m_TextureID);
        glTexStorage2D(m_Target, spec.mips, InternalFormat(spec.format), spec.width, spec.height);

        glTexParameteri(m_Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(m_Target, GL_TEXTURE_MIN_FILTER,
                        spec.mips > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTexParameteri(m_Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(m_Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if (m_Target == GL_TEXTURE_CUBE_MAP)
            glTexParameteri(m_Target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glGenFramebuffers(1, &m_FBO);
    }

    OpenGLRenderTexture::~OpenGLRenderTexture()
    {
        if (m_FBO) glDeleteFramebuffers(1, &m_FBO);
        if (m_TextureID) glDeleteTextures(1, &m_TextureID);
    }

    void OpenGLRenderTexture::BindAsTarget(unsigned int face, unsigned int mip)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

        if (m_Target == GL_TEXTURE_CUBE_MAP)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, m_TextureID, mip);
        else
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, m_TextureID, mip);

        unsigned int mipWidth = std::max(1u, m_Spec.width >> mip);
        unsigned int mipHeight = std::max(1u, m_Spec.height >> mip);
        glViewport(0, 0, mipWidth, mipHeight);
    }

    void OpenGLRenderTexture::Unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLRenderTexture::BindAsTexture(unsigned int slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(m_Target, m_TextureID);
    }

    void OpenGLRenderTexture::GenerateMips()
    {
        if (m_Spec.mips <= 1) return;
        glBindTexture(m_Target, m_TextureID);
        glGenerateMipmap(m_Target);
    }

}
