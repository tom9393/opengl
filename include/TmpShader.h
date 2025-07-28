#pragma once

#include "shader.h"

class TmpShader : public Shader
{
public:

    TmpShader();
    virtual bool Init();

    void setup();
    void cleanup();
};

