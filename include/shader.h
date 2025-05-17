#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <list>

class Shader
{
public:

    Shader();

    virtual ~Shader();

    virtual bool Init();

    void Enable();

    GLuint GetProgram() const { return id; }

protected:

    bool AddShader(GLenum ShaderType, const char* pFilename);

    bool Finalize();

    GLint GetUniformLocation(const char* pUniformName);

    GLuint id = 0;

private:

    void PrintUniformList();

    std::list<GLuint> shaderObjList;
};


#endif
