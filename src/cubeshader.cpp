#include "../include/cubeshader.h"

Cubeshader::Cubeshader()
{
}

bool Cubeshader::Init()
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

void Cubeshader::setup()
{
    setView();
}

void Cubeshader::cleanup()
{
}

