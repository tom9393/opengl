#ifndef DRAWABLE
#define DRAWABLE
#include "shader.h"

class IDrawable {

public:
    virtual void draw(Shader &shader) = 0;
};

#endif
