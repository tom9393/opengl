#ifndef SHADOWSHADER
#define SHADOWSHADER

#include "drawable.h"
#include "mainshader.h"
#include <glm/glm.hpp>
#include <glad/glad.h>

class Shadowshader : public Mainshader
{
public:

    Shadowshader();
    virtual bool Init();

    void setup();
    void cleanup();
};


#endif
