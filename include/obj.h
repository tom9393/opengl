#ifndef OBJ
#define OBJ

#include "drawable.h"
#include "model.h"
#include "shader.h"
#include <glm/fwd.hpp>

class Obj
{
public:
    Obj(IDrawable &drawable, glm::vec3 loc = glm::vec3(0.0f)) : drawable(&drawable), location(loc) {};
    glm::vec3 getLocation();
    glm::mat4 getMatrix();
    void draw(Shader &shader);

private:
    IDrawable* drawable;
    glm::vec3 location;
};

#endif
