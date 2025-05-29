#ifndef OLDSHADER_H
#define OLDSHADER_H

#include "shader.h"
#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
  
class OldShader : public Shader
{
public:
    OldShader();
    virtual bool Init();
};
  
#endif
