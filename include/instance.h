#ifndef INSTANCE
#define INSTANCE

#include "drawable.h"
#include "shader.h"
#include "transformable.h"
#include "preprocessor.h"


class Instance : public IDrawable, public Transformable
{
public:
    Instance(IDrawable &drawable, Transformable &transformable) : drawable(&drawable), Transformable(transformable) {};
    Instance(IDrawable &drawable, glm::vec3 location) : drawable(&drawable), Transformable(location) {};

    void draw(Shader &shader) override
    {
        Info("Drawing instance");
        shader.setMat4("model", getMatrix());
        drawable->draw(shader);
    }

    bool shouldDraw(Shader &shader) override
    {
        return drawable->shouldDraw(shader);
    }

private:
    IDrawable* drawable;
};

#endif
