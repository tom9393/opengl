#include "../include/mainshader.h"
#include <glm/glm.hpp>

Mainshader::Mainshader()
{
}

bool Mainshader::Init()
{
    if (!Shader::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "/home/tom/Coding/c++/opengl/shaders/default.vert")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "/home/tom/Coding/c++/opengl/shaders/default.frag")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    return true;
}

void Mainshader::setup()
{
    setView();
}

void Mainshader::cleanup()
{
}

