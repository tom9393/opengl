#pragma once

#include "pch.h"

namespace Engine
{

    // Progressive dual-filter bloom (downsample chain + additive tent upsample),
    // following the Call of Duty / Froyok approach. Self-contained: owns a mip
    // chain of float framebuffers driven with direct GL, so it does not require
    // the render graph's framebuffers to support mip levels.
    class Bloom
    {
    public:
        ~Bloom();

        // (Re)allocate the mip chain for the given full-resolution size. Cheap
        // no-op when the size is unchanged.
        void Resize(unsigned int width, unsigned int height);

        // Run the bloom chain over an HDR source texture and return the texture
        // id of the result (largest mip). Leaves no framebuffer bound.
        unsigned int Render(unsigned int srcTexture, float threshold);

    private:
        void Destroy();

        struct Mip
        {
            unsigned int Texture = 0;
            glm::vec2    Size     = { 0, 0 };
        };

        unsigned int      m_FBO   = 0;
        unsigned int      m_Width = 0, m_Height = 0;
        std::vector<Mip>  m_Mips;
        float             m_FilterRadius = 0.005f;
    };

}
