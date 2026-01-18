#pragma once

#include "../pch.h"

namespace Engine
{

    enum class TextureType
    {
        None = 0,
        Diffuse,
        Specular,
        Normal,
        Height
    };

    class Texture
    {
        public:
        virtual ~Texture() = default;
        virtual unsigned int GetWidth() const = 0;
        virtual unsigned int GetHeight() const = 0;

        virtual unsigned int GetID() const = 0;
        virtual std::string &GetPath() = 0;
        virtual TextureType GetType() const = 0;

        virtual void Bind(unsigned int slot) const = 0;
    };

    class Texture2D : public Texture
    {
        public:
        static std::shared_ptr<Texture2D> Create(const std::string &path, TextureType type);
    };

    class Texture3D : public Texture
    {
        public:
        static std::shared_ptr<Texture3D> Create(const std::string &path, TextureType type);
    };

    class BindlessTexture
    {
        public:
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
        static std::shared_ptr<BindlessTexture> Create();
    };

};
