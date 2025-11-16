#pragma once

#include "../../pch.h"

namespace Engine
{

    class Shader
    {
    public:

        virtual ~Shader() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual void AddShaders(const std::vector<std::string> &shaders) = 0;
        virtual void AddShaders(const std::initializer_list<std::string> &shaders) = 0;

        virtual const std::string& GetName() const = 0;

        static Shared<Shader> Create(const std::string &name);

        virtual void setBool(const std::string &name, bool value) const = 0;
        virtual void setInt(const std::string &name, int value) const = 0;
        virtual void setFloat(const std::string &name, float value) const = 0;
        virtual void setVec2(const std::string &name, const glm::vec2 &transform) const = 0;
        virtual void setVec3(const std::string &name, const glm::vec3 &transform) const = 0;
        virtual void setVec4(const std::string &name, const glm::vec4 &transform) const = 0;
        virtual void setMat2(const std::string &name, const glm::mat2 &transform) const = 0;
        virtual void setMat3(const std::string &name, const glm::mat3 &transform) const = 0;
        virtual void setMat4(const std::string &name, const glm::mat4 &transform) const = 0;
    };

    class ShaderLibrary
    {
        public:
        ShaderLibrary(const std::initializer_list<std::string> &shaders);

        void Add(const Shared<Shader> &shader);
        void AddToShader(const std::string &shader, const std::initializer_list<std::string> &paths);

        Shared<Shader> Get(const std::string &name);

        private:
        std::unordered_map<std::string, Shared<Shader>> m_Shaders;
    };

};
