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

void Cube::setLocation(glm::vec3 loc)
{
    location = loc;
}

void Cube::setRotationTarget(glm::vec3 target)
{
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 viewMatrix = glm::lookAt(location, target, up);

    glm::mat4 rotationMatrix = glm::inverse(viewMatrix);

    model = glm::mat4(1.0f);
    model = glm::translate(model, location);
    model *= rotationMatrix;  // Apply rotation to face the center
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::mat4 Cube::getModel()
{
    return model;
}

void Cube::draw(OldShader &shader)
{
    shader.setMat4("model", getModel());

    activate();
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    deactivate();
}
