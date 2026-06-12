#pragma once

#include "pch.h"
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
        static std::shared_ptr<Texture2D> Create(const TextureLoadData &data);

        virtual void Copy(uint32_t rendererID, uint32_t width, uint32_t height) = 0;
        virtual void ResizeCopy(uint32_t rendererID, uint32_t width, uint32_t height) = 0;

        // Reads the level-0 image back from the GPU as tightly-packed RGBA8 (width*height*4 bytes).
        virtual void ReadPixels(std::vector<unsigned char>& out) const = 0;
    };

    class Texture3D : public Texture
    {
        public:
        static std::shared_ptr<Texture3D> Create(const std::filesystem::path &path);
    };

    // For now unused
    class BindlessTexture
    {
        public:
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
        static std::shared_ptr<BindlessTexture> Create();
    };

};
