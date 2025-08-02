#pragma once

#include "../pch.h"

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
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;

    virtual uint32_t GetID() const = 0;

    virtual void Bind(uint32_t slot) const = 0;
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
