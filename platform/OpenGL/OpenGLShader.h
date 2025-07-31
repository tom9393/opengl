#pragma once

#include "../../pch.h"
#include "../../include/Shader.h"

class OpenGLShader : public Shader
{
public:

    OpenGLShader(const char* vertexSrc, const char* fragmentSrc);

    ~OpenGLShader();

    void Bind() const override;
    void Unbind() const override;

    GLuint GetProgram() const { return id; }

    // utility uniform functions
    void setBool(const std::string &name, bool value) const;  
    void setInt(const std::string &name, int value) const;   
    void setFloat(const std::string &name, float value) const;
    void setVec2(const std::string &name, const glm::vec2 &transform) const;
    void setVec3(const std::string &name, const glm::vec3 &transform) const;
    void setVec4(const std::string &name, const glm::vec4 &transform) const;
    void setMat2(const std::string &name, const glm::mat2 &transform) const;
    void setMat3(const std::string &name, const glm::mat3 &transform) const;
    void setMat4(const std::string &name, const glm::mat4 &transform) const;

protected:

    bool AddShader(GLenum ShaderType, const char* pFilename);

    bool Finalize();

    GLint GetUniformLocation(const char* pUniformName);

    GLuint id = 0;

private:

    void PrintUniformList();

    std::list<GLuint> shaderObjList;
};

