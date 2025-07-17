#ifndef OBJ
#define OBJ

#include "drawable.h"
#include "transform.h"
#include "shader.h"
#include <glm/fwd.hpp>

class Obj : public Transform, public IDrawable
{
public:
    Obj(IDrawable &drawable, glm::vec3 location = glm::vec3(0.0f), glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f), glm::vec3 velocity = glm::vec3(0.0f), glm::vec3 angularVelocity = glm::vec3(0.0f)) : drawable(&drawable), Transform(location, rotation, scale, velocity, angularVelocity) {};
    
    void draw(Shader &shader);
    bool shouldDraw(Shader &shader);

private:
    IDrawable* drawable;
};

#endif
