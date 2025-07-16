#include "../include/obj.h"

void Obj::setLocation(glm::vec3 &loc)
{
    location.setPosition(loc);
}

void Obj::rotate(glm::vec3 &rotation)
{
    location.rotate(rotation);
}

glm::mat4 Obj::getMatrix()
{
    return location.getMatrix();
}

Transform* Obj::getTransform()
{
    return &location;
}

void Obj::draw(Shader &shader)
{
    shader.setMat4("model", getMatrix());
    drawable->draw(shader);
}

bool Obj::shouldDraw(Shader &shader)
{
    return drawable->shouldDraw(shader);
}

void Obj::update(float deltaTime) {
    location.move(velocity * deltaTime);
    location.rotate(angularVelocity * deltaTime);
}
