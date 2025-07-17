#include "../include/cube.h"
#include "../include/preprocessor.h"

Cube::Cube()
{
    init();
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

void Cube::setLightLocation(glm::vec3 *loc)
{
    if (associatedLight != nullptr) {
        associatedLight->setPosition(*loc);
        associatedLight->setDirection(associatedLight->getDirectionToTarget(associatedLight->getMatrix()));
    }
}

void Cube::setLight(SpotLight* light)
{
    associatedLight = light;
}

void Cube::draw(Shader &shader)
{
    Info("Rendering a cube");
    activate();
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    deactivate();
}

bool Cube::shouldDraw(Shader &shader)
{
    return true;
}
