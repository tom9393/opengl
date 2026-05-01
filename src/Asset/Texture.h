#pragma once

#include "../pch.h"
#include "Asset.h"

namespace Engine
{

    struct TextureLoadData;

    class Texture : public Asset
    {
        public:
        virtual ~Texture() = default;
        virtual unsigned int GetWidth() const = 0;
        virtual unsigned int GetHeight() const = 0;

        virtual unsigned int GetID() const = 0;
        virtual std::filesystem::path &GetPath() = 0;

        virtual void Bind(unsigned int slot) = 0;
    };

    class Texture2D : public Texture
    {
        public:
        static std::shared_ptr<Texture2D> Create(const std::filesystem::path &path);
        static std::shared_ptr<Texture2D> Create(const TextureLoadData &data);
        static std::shared_ptr<Texture2D> Create(uint32_t rendererID, uint32_t width, uint32_t height); // Mostly to copy from framebuffer
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
