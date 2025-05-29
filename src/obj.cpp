#include "../include/obj.h"

glm::vec3 Obj::getLocation()
{
    return location;
}

glm::mat4 Obj::getMatrix()
{
    glm::mat4 m = glm::mat4(1.0f);
    return glm::translate(m, location);
}

void Obj::draw(Shader &shader)
{
    shader.setMat4("model", getMatrix());
    model->Draw(shader);
}
