#ifndef ISHADER
#define ISHADER

#include "light.h"

class IShader : public Shader
{
public:
    virtual void setLight(unsigned int index, ILight &light) = 0;
};

#endif
