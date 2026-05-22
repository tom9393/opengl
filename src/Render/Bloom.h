#pragma once

#include "pch.h"
#include "RenderTexture.h"

namespace Engine
{

    // Progressive dual-filter bloom (downsample chain + additive tent upsample),
    // following the Call of Duty / Froyok approach. Owns a mip chain of float
    // RenderTextures; all GPU work goes through RenderCommand and RenderTexture,
    // so the class carries no render-API specifics.
    class Bloom
    {
    public:
        // (Re)allocate the mip chain for the given full-resolution size. Cheap
        // no-op when the size is unchanged.
        void Resize(unsigned int width, unsigned int height);

        // Run the bloom chain over an HDR source texture and return the texture
        // id of the result (largest mip). Leaves no framebuffer bound.
        unsigned int Render(unsigned int srcTexture, float threshold);

    private:
        struct Mip
        {
            Shared<RenderTexture> texture;
            glm::vec2             size = { 0, 0 };
        };

        unsigned int      m_Width = 0, m_Height = 0;
        std::vector<Mip>  m_Mips;
        float             m_FilterRadius = 0.005f;
    };

}
