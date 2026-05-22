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

    // A sampleable texture that can also be rendered into (it owns an internal
    // framebuffer). Unlike the render-graph Framebuffer - which models a fixed-size
    // set of equally-sized attachments drawn as a whole - a RenderTexture targets a
    // single color image one cube face / mip level at a time. It backs offscreen
    // sub-renderers such as the bloom mip chain and the IBL cubemaps.
    class RenderTexture
    {
    public:
        virtual ~RenderTexture() = default;

        // Make this the active render target. For a cubemap, selects the face;
        // for a 2D texture the face is ignored. Attaches the chosen mip level and
        // sets the viewport to that mip's size.
        virtual void BindAsTarget(unsigned int face = 0, unsigned int mip = 0) = 0;
        virtual void Unbind() const = 0;

        // Bind the underlying texture to a sampler slot.
        virtual void BindAsTexture(unsigned int slot) const = 0;

        // Regenerate the mip chain from mip 0 (no-op for single-mip textures).
        virtual void GenerateMips() = 0;

        virtual unsigned int GetID() const = 0;
        virtual unsigned int GetWidth() const = 0;
        virtual unsigned int GetHeight() const = 0;

        static Shared<RenderTexture> Create(const RenderTextureSpecification& spec);
    };

}
