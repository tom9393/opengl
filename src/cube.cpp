#include "../include/cube.h"

Cube::Cube()
{
}

void Cube::init()
{

    vao.Bind();
    vao.LinkVBO(vbo);
    vao.Unbind();
}

void Cube::activate()
{
    vao.Bind();
    ebo.Bind();
}

void Cube::deactivate()
{
    vao.Unbind();
    ebo.Unbind();
}
