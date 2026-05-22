#include "Bloom.h"

#include "Shader.h"
#include "RenderCommand.h"

namespace Engine
{

    namespace { constexpr unsigned int MAX_MIPS = 6; }

    void Bloom::Resize(unsigned int width, unsigned int height)
    {
        if (width == m_Width && height == m_Height && !m_Mips.empty()) return;
        if (width == 0 || height == 0) return;

        m_Mips.clear();
        m_Width = width;
        m_Height = height;

        // First mip is half resolution; each subsequent mip halves again.
        glm::vec2 mipSize = { (float)width, (float)height };
        for (unsigned int i = 0; i < MAX_MIPS; ++i)
        {
            mipSize *= 0.5f;
            if (mipSize.x < 1.0f || mipSize.y < 1.0f) break;

            RenderTextureSpecification spec;
            spec.width  = (unsigned int)mipSize.x;
            spec.height = (unsigned int)mipSize.y;
            spec.type   = RenderTextureType::Texture2D;
            spec.format = RenderTextureFormat::RGBA16F;
            spec.mips   = 1;

            Mip mip;
            mip.size    = mipSize;
            mip.texture = RenderTexture::Create(spec);
            m_Mips.push_back(mip);
        }
    }

    unsigned int Bloom::Render(unsigned int srcTexture, float threshold)
    {
        if (m_Mips.empty()) return 0;

        // --- Downsample ---
        Shared<Shader> down = ShaderLibrary::Get("BloomDownsample");
        down->Bind();
        down->SetInt("u_SrcTexture", 0);
        down->SetFloat("u_Threshold", threshold);

        glm::vec2 srcResolution = { (float)m_Width, (float)m_Height };
        for (unsigned int i = 0; i < m_Mips.size(); ++i)
        {
            const Mip& mip = m_Mips[i];
            down->SetVec2("u_SrcResolution", srcResolution);
            down->SetInt("u_FirstMip", i == 0 ? 1 : 0);

            mip.texture->BindAsTarget();
            if (i == 0) RenderCommand::BindTexture2D(0, srcTexture);
            else        m_Mips[i - 1].texture->BindAsTexture(0);
            RenderCommand::DrawFullscreenQuad();

            srcResolution = mip.size;
        }

        // --- Upsample (additive) ---
        Shared<Shader> up = ShaderLibrary::Get("BloomUpsample");
        up->Bind();
        up->SetInt("u_SrcTexture", 0);
        up->SetFloat("u_FilterRadius", m_FilterRadius);

        RenderCommand::SetAdditiveBlend(true);
        for (size_t i = m_Mips.size() - 1; i > 0; --i)
        {
            m_Mips[i].texture->BindAsTexture(0);
            m_Mips[i - 1].texture->BindAsTarget();
            RenderCommand::DrawFullscreenQuad();
        }
        RenderCommand::SetAdditiveBlend(false);

        m_Mips.front().texture->Unbind();
        return m_Mips.front().texture->GetID();
    }

}
