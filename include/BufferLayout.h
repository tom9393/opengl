#pragma once

#include "Texture.h"
#include <initializer_list>
enum class ShaderDataType
{
    None = 0,
    Float, Float2, Float3, Float4,
    Int, Int2, Int3, Int4,
    Mat3, Mat4,
    Bool
};

static unsigned int ShaderDataTypeSize(ShaderDataType type)
{
    switch (type)
    {
        case ShaderDataType::Float: return 4;
        case ShaderDataType::Float2: return 4 * 2;
        case ShaderDataType::Float3: return 4 * 3;
        case ShaderDataType::Float4: return 4 * 4;
        case ShaderDataType::Int: return 4;
        case ShaderDataType::Int2: return 4 * 2;
        case ShaderDataType::Int3: return 4 * 3;
        case ShaderDataType::Int4: return 4 * 4;
        case ShaderDataType::Bool: return 1;
        case ShaderDataType::Mat3: return 4 * 3 * 3;
        case ShaderDataType::Mat4: return 4 * 4 * 4;
    }
    return 0;
};

struct BufferElement
{
    std::string Name;
    ShaderDataType Type;
    unsigned int Offset;
    unsigned int Size;
    bool Normalized;

    BufferElement() {}
    BufferElement(ShaderDataType type, const std::string &name, bool normalized = false) : Name(name), Type(type), Normalized(normalized), Size(ShaderDataTypeSize(type)), Offset(0) {}

    unsigned int GetComponentCount() const
    {
        switch (Type)
        {
            case ShaderDataType::Float: return 1;
            case ShaderDataType::Float2: return 2;
            case ShaderDataType::Float3: return 3;
            case ShaderDataType::Float4: return 4;
            case ShaderDataType::Int: return 1;
            case ShaderDataType::Int2: return 2;
            case ShaderDataType::Int3: return 3;
            case ShaderDataType::Int4: return 4;
            case ShaderDataType::Bool: return 1;
            case ShaderDataType::Mat3: return 3 * 3;
            case ShaderDataType::Mat4: return 4 * 4;
        }
        return 0;
    }
};

class BufferLayout
{
    public:
    BufferLayout() {}
    BufferLayout(const std::initializer_list<BufferElement> &elements) : m_Elements(elements) {
        CalculateOffsetAndStride();
    }
    BufferLayout(const std::vector<BufferElement> &elements) : m_Elements(elements) {
        CalculateOffsetAndStride();
    }
    unsigned int GetStride() const { return m_Stride; }
    const std::vector<BufferElement>& GetElements() const { return m_Elements; }
    std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
    std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
    bool operator==(const BufferLayout &layout) { return this->m_Stride == layout.m_Stride; }
    private:
    void CalculateOffsetAndStride()
    {
        unsigned int offset = 0;
        m_Stride = 0;
        for (auto& element : m_Elements)
        {
            element.Offset = offset;
            offset += element.Size;
            m_Stride += element.Size;
        }
    }
    std::vector<BufferElement> m_Elements;
    unsigned int m_Stride = 0;
};

class TextureLayout
{
    public:
    TextureLayout(const std::vector<std::shared_ptr<Texture2D>> &textures)
    {
        for (auto &texture : textures)
        {
            m_Textures[texture->GetType()].emplace_back(texture);
        }
        m_Texture_Keys[TextureType::Diffuse] = "material.diffuse";
        m_Texture_Keys[TextureType::Specular] = "material.specular";
        m_Texture_Keys[TextureType::Height] = "material.height";
        m_Texture_Keys[TextureType::Normal] = "material.normal";
    }

    std::vector<std::shared_ptr<Texture2D>> &GetByType(TextureType type)
    {
        return m_Textures[type];
    }

    unsigned int GetCountByType(TextureType type)
    {
        return m_Textures[type].size();
    }

    std::string &GetStringByType(TextureType type) { return m_Texture_Keys[type]; }

    std::unordered_map<TextureType, std::vector<std::shared_ptr<Texture2D>>>::const_iterator begin() const { return m_Textures.begin(); }
    std::unordered_map<TextureType, std::vector<std::shared_ptr<Texture2D>>>::const_iterator end() const { return m_Textures.end(); }

    private:
    std::unordered_map<TextureType, std::vector<std::shared_ptr<Texture2D>>> m_Textures;
    std::unordered_map<TextureType, std::string> m_Texture_Keys;
};
