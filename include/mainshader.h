#ifndef LIGHTING
#define LIGHTING

#include "ishader.h"

class Mainshader : public ISetViewPosition
{
public:

    Mainshader();

    virtual bool Init();

    void setup();
    void cleanup();
};

#endif
