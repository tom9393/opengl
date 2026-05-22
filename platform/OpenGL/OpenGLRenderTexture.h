#pragma once

#include "Render/RenderTexture.h"

namespace Engine
{

    class OpenGLRenderTexture : public RenderTexture
    {
    public:
        OpenGLRenderTexture(const RenderTextureSpecification& spec);
        ~OpenGLRenderTexture();

        void BindAsTarget(unsigned int face = 0, unsigned int mip = 0) override;
        void Unbind() const override;
        void BindAsTexture(unsigned int slot) const override;
        void GenerateMips() override;

        unsigned int GetID() const override { return m_TextureID; }
        unsigned int GetWidth() const override { return m_Spec.width; }
        unsigned int GetHeight() const override { return m_Spec.height; }

    private:
        RenderTextureSpecification m_Spec;
        unsigned int m_FBO = 0;
        unsigned int m_TextureID = 0;
        unsigned int m_Target = 0; // GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP
    };

}
