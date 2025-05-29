#include "../include/shaderClass.h"

OldShader::OldShader()
{
}

bool OldShader::Init()
{
    if (!Shader::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "/home/tom/Coding/c++/opengl/shaders/light.vert")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "/home/tom/Coding/c++/opengl/shaders/light.frag")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    return true;
}
