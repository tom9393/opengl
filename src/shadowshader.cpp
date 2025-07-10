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

void Shadowshader::setup()
{
}
void Shadowshader::draw()
{

    for (unsigned int i = 0; i < lights.size(); i++) {
        IMappedLight* light = lights[i];
        light->setMatrix(*this);
        for (unsigned int j = 0; j < objs.size(); j++)
        {
            Obj *obj = objs[j];
            obj->draw(*this);
        }
    }
}
void Shadowshader::cleanup()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 1600, 1600);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
