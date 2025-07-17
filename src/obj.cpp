#include "../include/obj.h"

void Obj::draw(Shader &shader)
{
    shader.setMat4("model", getMatrix());
    drawable->draw(shader);
}

bool Obj::shouldDraw(Shader &shader)
{
    return drawable->shouldDraw(shader);
}

