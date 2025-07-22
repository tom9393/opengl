#ifndef BATCH
#define BATCH

#include "../pch.h"

#include "shader.h"
#include "drawable.h"

class Batch {
public:
    Batch(unsigned int flags, std::vector<IDrawable*> drawables, Shader* shader) : flags(flags), drawables(drawables), shader(shader) {};
    void render();

    Shader* shader;
private:
    unsigned int flags;
    std::vector<IDrawable*> drawables;
};

#endif
