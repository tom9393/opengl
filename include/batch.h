#ifndef BATCH
#define BATCH

#include "shader.h"
#include "drawable.h"
#include <vector>

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
