#pragma once

#include "../../pch.h"
#include "../../src/Render/Shader.h"

namespace Engine
{

    class OpenGLShader : public Shader
    {
    public:

        OpenGLShader(const std::string &name);

        ~OpenGLShader();

        void Bind() const override;
        void Unbind() const override;

        const std::string& GetName() const override { return m_Name; }

        void AddShaders(const std::vector<std::string> &shaders) override;
        void AddShaders(const std::initializer_list<std::string> &shaders) override;

        GLuint GetProgram() const { return id; }

        // utility uniform functions
        void setBool(const std::string &name, bool value) const override;
        void setInt(const std::string &name, int value) const override;
        void setFloat(const std::string &name, float value) const override;
        void setVec2(const std::string &name, const glm::vec2 &transform) const override;
        void setVec3(const std::string &name, const glm::vec3 &transform) const override;
        void setVec4(const std::string &name, const glm::vec4 &transform) const override;
        void setMat2(const std::string &name, const glm::mat2 &transform) const override;
        void setMat3(const std::string &name, const glm::mat3 &transform) const override;
        void setMat4(const std::string &name, const glm::mat4 &transform) const override;

    protected:

        GLint GetUniformLocation(const char* pUniformName);
        GLenum GetShaderFromExtension(const std::string &path);
        GLuint id = 0;

    private:

        void PrintUniformList();
        bool AddShader(const std::string &file);
        bool Finalize();
        std::list<GLuint> shaderObjList;
        std::string m_Name;
    };

};
