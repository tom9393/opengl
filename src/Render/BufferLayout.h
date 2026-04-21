#pragma once

#include "../pch.h"
#include "../Asset/Texture.h"
#include "../Asset/MaterialSpecification.h"

namespace Engine
{

    enum class ShaderDataType
    {
        None = 0,
        Float, Float2, Float3, Float4,
        Int, Int2, Int3, Int4,
        Mat3, Mat4,
        Bool
    };

    enum class VertexAttribute {
        None = 0,
        Vertex,
        Normal,
        Texture,
        Tangent,
        Bitangent,
        Color,
        Instance
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

        bool Enabled;

        BufferElement() {}
        BufferElement(ShaderDataType type, const std::string &name, bool normalized = false, bool enabled = true) : Name(name), Type(type), Normalized(normalized), Size(ShaderDataTypeSize(type)), Offset(0), Enabled(enabled) {}

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

    class VertexLayout
    {
        public:
        VertexLayout() {};

        void AddVertexAttrib(VertexAttribute attrib, ShaderDataType type, const std::string &str)
        {
            if (m_Contains.find(attrib) != m_Contains.end()) return;

            m_Attribs.emplace_back(attrib);
            m_Elements.emplace_back(type, str);
            m_Contains[attrib] = true;
        }

        std::vector<VertexAttribute> GetAttribs() { return m_Attribs; };
        std::vector<BufferElement> GetElements() { return m_Elements; };

        private:
        std::vector<VertexAttribute> m_Attribs;
        std::vector<BufferElement> m_Elements;
        std::unordered_map<VertexAttribute, bool> m_Contains;

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
        TextureLayout()
        {
            m_Texture_Keys[TextureType::Diffuse] = "pbr.albedoMap";
            m_Texture_Keys[TextureType::Normal] = "pbr.normalMap";
            m_Texture_Keys[TextureType::Metallic] = "pbr.metallicMap";
            m_Texture_Keys[TextureType::Roughness] = "pbr.roughnessMap";
            m_Texture_Keys[TextureType::AO] = "pbr.aoMap";
        }

        void AddTexture(TextureType type, Shared<Texture2D> texture)
        {
            m_Textures[type].emplace_back(texture);
        }

        std::vector<Shared<Texture2D>> &GetByType(TextureType type)
        {
            return m_Textures[type];
        }

        unsigned int GetCountByType(TextureType type)
        {
            return m_Textures[type].size();
        }

        std::string &GetStringByType(TextureType type) { return m_Texture_Keys[type]; }

        std::unordered_map<TextureType, std::vector<Shared<Texture2D>>>::const_iterator begin() const { return m_Textures.begin(); }
        std::unordered_map<TextureType, std::vector<Shared<Texture2D>>>::const_iterator end() const { return m_Textures.end(); }

        private:
        std::unordered_map<TextureType, std::vector<Shared<Texture2D>>> m_Textures;
        std::unordered_map<TextureType, std::string> m_Texture_Keys;
    };
};
