#pragma once

#include "../pch.h"
#include "Asset.h"

namespace Engine
{

    enum class TextureType
    {
        None = 0,
        Albedo,    // PBR: Base color
        Normal,    // PBR: Surface detail
        Metallic,  // PBR: Metalness
        Roughness, // PBR: Micro-surface detail
        AO,        // PBR: Ambient Occlusion
        Height,    // Often used for parallax
        Diffuse,   // Legacy
        Specular   // Legacy
    };

    class Texture : public Asset
    {
        public:
        virtual ~Texture() = default;
        virtual unsigned int GetWidth() const = 0;
        virtual unsigned int GetHeight() const = 0;

        virtual unsigned int GetID() const = 0;
        virtual std::filesystem::path &GetPath() = 0;

        virtual void Bind(unsigned int slot) const = 0;
    };

    class Texture2D : public Texture
    {
        public:
        static std::shared_ptr<Texture2D> Create(const std::filesystem::path &path);
    };

    class Texture3D : public Texture
    {
        public:
        static std::shared_ptr<Texture3D> Create(const std::filesystem::path &path);
    };

    class BindlessTexture
    {
        public:
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
        static std::shared_ptr<BindlessTexture> Create();
    };

};
