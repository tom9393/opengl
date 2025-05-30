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
{}
void Cubeshader::draw()
{
    for (unsigned int i = 0; i < cubes.size(); i++)
    {
        Cube *cube = cubes[i];
        cube->setRotationTarget(glm::vec3(0.0f));
        cube->draw(*this);
    }
}
void Cubeshader::cleanup()
{}

void Cubeshader::addCube(Cube* cube)
{
    cubes.push_back(cube);
}
