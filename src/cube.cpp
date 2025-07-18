#include "../include/cube.h"
#include "../include/preprocessor.h"

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

void Cube::draw(Shader &shader)
{
    Info("Rendering a cube");
    activate();
    shader.setMat4("model", getMatrix());
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    deactivate();
}

bool Cube::shouldDraw(Shader &shader)
{
    return true;
}
