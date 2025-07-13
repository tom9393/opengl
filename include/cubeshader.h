#ifndef CUBESHADER_H
#define CUBESHADER_H

#include "drawable.h"
#include "ishader.h"
#include <glad/glad.h>
#include <glm/fwd.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
  
class Cubeshader : public ISetViewPosition
{
public:
    Cubeshader();
    virtual bool Init();

    void setup();
    void draw(std::vector<IDrawable*> &drawables);
    void cleanup();

};
  
#endif
