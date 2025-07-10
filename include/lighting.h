#ifndef LIGHTING
#define LIGHTING

#include "ishader.h"
#include "mappedlight.h"
#include "obj.h"
#include "shader.h"
#include "light.h"
#include "shadowmap.h"
#include <glm/glm.hpp>
#include <vector>

class Lighting : public IShader
{
public:

    Lighting();

    virtual bool Init();

    void setup();
    void draw();
    void cleanup();


    void addObj(Obj *object);
    void addLight(IMappedLight *mappedLight);

protected:
    std::vector<IMappedLight*> lights;
    std::vector<Obj*> objs;
    void setLight(unsigned int index, ILight &light);
private:
    void setSpotlight(unsigned int index, SpotLight &light);
    void setPointlight(unsigned int index, PointLight &light);
    void setDirlight(unsigned int index, DirLight &light);
};

#endif
