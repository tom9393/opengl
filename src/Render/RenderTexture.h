#pragma once

#include "pch.h"

namespace Engine
{

    enum class RenderTextureFormat
    {
        RGBA16F,
        RG16F
    };

    enum class RenderTextureType
    {
        Texture2D,
        Cubemap
    };

    struct RenderTextureSpecification
    {
        unsigned int width = 0, height = 0;
        RenderTextureType type = RenderTextureType::Texture2D;
        RenderTextureFormat format = RenderTextureFormat::RGBA16F;
        unsigned int mips = 1; // number of mip levels to allocate
    };

    // Sampleable texture that can be rendered into (owns an internal framebuffer)
    // Render graph Framebuffer -> graph owned RenderTargets drawn as a whole
    // RenderTexture -> single color image one cube face / mip level at a time
    // Backs offscreen sub renderers such as the bloom mip chain and the IBL cubemaps
    class RenderTexture
    {
    public:
        virtual ~RenderTexture() = default;

        // For cubemap selects the face, ignored for 2D textures
        // Attaches chosen mip level and sets the viewport to that mip's size
        virtual void BindAsTarget(unsigned int face = 0, unsigned int mip = 0) = 0;
        virtual void Unbind() const = 0;

        // Bind to a sampler slot
        virtual void BindAsTexture(unsigned int slot) const = 0;

        // Regenerate the mip chain from mip 0 (no-op for single-mip textures)
        virtual void GenerateMips() = 0;

        virtual unsigned int GetID() const = 0;
        virtual unsigned int GetWidth() const = 0;
        virtual unsigned int GetHeight() const = 0;

        static Shared<RenderTexture> Create(const RenderTextureSpecification& spec);
    };

}
