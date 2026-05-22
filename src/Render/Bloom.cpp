#include "Bloom.h"

#include "Shader.h"
#include "Renderer.h"

namespace Engine
{

    namespace { constexpr unsigned int MAX_MIPS = 6; }

    Bloom::~Bloom()
    {
        Destroy();
    }

    void Bloom::Destroy()
    {
        for (auto& mip : m_Mips)
            if (mip.Texture) glDeleteTextures(1, &mip.Texture);
        m_Mips.clear();
        if (m_FBO) { glDeleteFramebuffers(1, &m_FBO); m_FBO = 0; }
    }

    void Bloom::Resize(unsigned int width, unsigned int height)
    {
        if (width == m_Width && height == m_Height && m_FBO) return;
        if (width == 0 || height == 0) return;

        Destroy();
        m_Width = width;
        m_Height = height;

        glGenFramebuffers(1, &m_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

        // First mip is half resolution; each subsequent mip halves again.
        glm::vec2 mipSize = { (float)width, (float)height };
        for (unsigned int i = 0; i < MAX_MIPS; ++i)
        {
            mipSize *= 0.5f;
            if (mipSize.x < 1.0f || mipSize.y < 1.0f) break;

            Mip mip;
            mip.Size = mipSize;
            glGenTextures(1, &mip.Texture);
            glBindTexture(GL_TEXTURE_2D, mip.Texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, (int)mipSize.x, (int)mipSize.y, 0, GL_RGB, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            m_Mips.push_back(mip);
        }

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               m_Mips.empty() ? 0 : m_Mips[0].Texture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    unsigned int Bloom::Render(unsigned int srcTexture, float threshold)
    {
        if (m_Mips.empty()) return 0;

        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glActiveTexture(GL_TEXTURE0);

        // --- Downsample ---
        Shared<Shader> down = ShaderLibrary::Get("BloomDownsample");
        down->Bind();
        down->SetInt("u_SrcTexture", 0);
        down->SetFloat("u_Threshold", threshold);

        glm::vec2 srcResolution = { (float)m_Width, (float)m_Height };
        unsigned int srcTex = srcTexture;
        for (unsigned int i = 0; i < m_Mips.size(); ++i)
        {
            const Mip& mip = m_Mips[i];
            down->SetVec2("u_SrcResolution", srcResolution);
            down->SetInt("u_FirstMip", i == 0 ? 1 : 0);

            glViewport(0, 0, (int)mip.Size.x, (int)mip.Size.y);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.Texture, 0);

            glBindTexture(GL_TEXTURE_2D, srcTex);
            Renderer::DrawFullscreenQuad();

            srcResolution = mip.Size;
            srcTex = mip.Texture;
        }

        // --- Upsample (additive) ---
        Shared<Shader> up = ShaderLibrary::Get("BloomUpsample");
        up->Bind();
        up->SetInt("u_SrcTexture", 0);
        up->SetFloat("u_FilterRadius", m_FilterRadius);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);

        for (size_t i = m_Mips.size() - 1; i > 0; --i)
        {
            const Mip& mip       = m_Mips[i];
            const Mip& nextMip   = m_Mips[i - 1];

            glBindTexture(GL_TEXTURE_2D, mip.Texture);
            glViewport(0, 0, (int)nextMip.Size.x, (int)nextMip.Size.y);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nextMip.Texture, 0);
            Renderer::DrawFullscreenQuad();
        }

        glDisable(GL_BLEND);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return m_Mips[0].Texture;
    }

}
