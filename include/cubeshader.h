#ifndef CUBESHADER_H
#define CUBESHADER_H

#include "cube.h"
#include "mainshader.h"
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
#include <vector>
  
class Cubeshader : public Mainshader
{
public:
    Cubeshader();
    virtual bool Init();

    void setup();
    void draw();
    void cleanup();

    void addCube(Cube *cube);

private:
    std::vector<Cube*> cubes;
};
  
#endif
