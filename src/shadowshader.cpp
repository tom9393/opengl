#include "../include/shadowshader.h"

Shadowshader::Shadowshader()
{
}

bool Shadowshader::Init()
{
    if (!Shader::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "/home/tom/Coding/c++/opengl/shaders/shadow.vert")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "/home/tom/Coding/c++/opengl/shaders/shadow.frag")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    return true;
}
