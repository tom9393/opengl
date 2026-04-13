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
        const std::vector<UniformInfo> &GetActiveUniforms() const override { return m_ActiveUniforms; };
        const std::vector<AttributeInfo> &GetActiveAttributes() const override { return m_ActiveAttributes; };

        // utility uniform functions
        void SetBool(const std::string &name, bool value) const override;
        void SetInt(const std::string &name, int value) const override;
        void SetFloat(const std::string &name, float value) const override;
        void SetVec2(const std::string &name, const glm::vec2 &transform) const override;
        void SetVec3(const std::string &name, const glm::vec3 &transform) const override;
        void SetVec4(const std::string &name, const glm::vec4 &transform) const override;
        void SetMat2(const std::string &name, const glm::mat2 &transform) const override;
        void SetMat3(const std::string &name, const glm::mat3 &transform) const override;
        void SetMat4(const std::string &name, const glm::mat4 &transform) const override;

    protected:

        GLint GetUniformLocation(const char* pUniformName);
        GLenum GetShaderFromExtension(const std::string &path);
        GLuint id = 0;

    private:

        void ExtractActiveAttributesAndUniforms();
        bool AddShader(const std::string &file);
        bool Finalize();
        std::list<GLuint> shaderObjList;
        std::string m_Name;
        std::vector<UniformInfo> m_ActiveUniforms;
        std::vector<AttributeInfo> m_ActiveAttributes;
    };

};
