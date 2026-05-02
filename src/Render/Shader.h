#pragma once

#include "pch.h"

namespace Engine
{

    struct UniformInfo
    {
        std::string Name;
        uint32_t Type;
        int Size;
    };

    struct AttributeInfo
    {
        std::string Name;
        uint32_t Type;    // GLenum (e.g., GL_FLOAT_VEC3)
        int Size;         // Number of elements (e.g., 1 for vec3, >1 for arrays)
        int Location;     // The layout(location = X) index
    };

    struct BufferBlockInfo
    {
        std::string Name;
        int BindingPoint;
        int SizeBytes;
    };

    class Shader
    {
    public:

        virtual ~Shader() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual void AddShaders(const std::vector<std::string> &shaders) = 0;
        virtual void AddShaders(const std::initializer_list<std::string> &shaders) = 0;

        virtual const std::string& GetName() const = 0;
        virtual const std::vector<UniformInfo> &GetActiveUniforms() const = 0;
        virtual const std::vector<AttributeInfo> &GetActiveAttributes() const = 0;
        virtual const std::vector<BufferBlockInfo>& GetActiveUBOs() const = 0;
        virtual const std::vector<BufferBlockInfo>& GetActiveSSBOs() const = 0;

        static Shared<Shader> Create(const std::string &name);

        virtual void SetBool(const std::string &name, bool value) const = 0;
        virtual void SetInt(const std::string &name, int value) const = 0;
        virtual void SetFloat(const std::string &name, float value) const = 0;
        virtual void SetVec2(const std::string &name, const glm::vec2 &transform) const = 0;
        virtual void SetVec3(const std::string &name, const glm::vec3 &transform) const = 0;
        virtual void SetVec4(const std::string &name, const glm::vec4 &transform) const = 0;
        virtual void SetMat2(const std::string &name, const glm::mat2 &transform) const = 0;
        virtual void SetMat3(const std::string &name, const glm::mat3 &transform) const = 0;
        virtual void SetMat4(const std::string &name, const glm::mat4 &transform) const = 0;
    };

    class ShaderLibrary
    {
        public:
        ShaderLibrary();
        ~ShaderLibrary();

        static void Init();
        static void Shutdown();

        static void Add(const Shared<Shader> &shader);
        static void AddToShader(const std::string &shader, const std::initializer_list<std::string> &paths);

        static Shared<Shader> Get(const std::string &name);
        static std::unordered_map<std::string, Shared<Shader>> &GetShaders() { return s_Shaders; };

        private:
        static std::unordered_map<std::string, Shared<Shader>> s_Shaders;
    };

};
