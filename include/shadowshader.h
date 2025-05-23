#ifndef SHADOWSHADER
#define SHADOWSHADER

#include "shader.h"
#include <glm/glm.hpp>
#include <glad/glad.h>

class Shadowshader : public Shader
{
public:

    Shadowshader();
    virtual bool Init();
};


#endif
