#ifndef CUBESHADER_H
#define CUBESHADER_H

#include "../pch.h"

#include "drawable.h"
#include "ishader.h"
  
class Cubeshader : public ISetViewPosition
{
public:
    Cubeshader();
    virtual bool Init();

    void setup();
    void cleanup();

};
  
#endif
