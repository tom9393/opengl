#ifndef DRAWABLE
#define DRAWABLE

class Shader;

class IDrawable {

public:
    virtual void draw(Shader &shader) = 0;
    virtual bool shouldDraw(Shader &shader) = 0;
};

#endif
