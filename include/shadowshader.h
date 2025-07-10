#ifndef SHADOWSHADER
#define SHADOWSHADER

#include "light.h"
#include "lighting.h"
#include "obj.h"
#include "shadowmap.h"
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

class Shadowshader : public Lighting
{
public:

    Shadowshader();
    virtual bool Init();

    void setup();
    void draw();
    void cleanup();
};


#endif
