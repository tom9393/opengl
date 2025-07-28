#include "../include/TmpShader.h"

TmpShader::TmpShader()
{
}

bool TmpShader::Init()
{
    if (!Shader::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "/home/tom/Coding/c++/opengl/shaders/tmp.vert")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "/home/tom/Coding/c++/opengl/shaders/tmp.frag")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    return true;
}

void TmpShader::setup()
{
}
void TmpShader::cleanup()
{
}
