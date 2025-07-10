#ifndef LIGHTING
#define LIGHTING

#include "obj.h"
#include "shader.h"
#include "light.h"
#include "shadowmap.h"
#include <glm/glm.hpp>
#include <vector>

class Lighting : public Shader
{
public:

    Lighting();

    virtual bool Init();

    // before run
    void setup();
    void draw();
    void cleanup();

    void setSpotlight(unsigned int index, SpotLight &light);
    void setPointlight(unsigned int index, PointLight &light);
    void setDirlight(unsigned int index, DirLight &light);

    void addShadowMap(IShadowMap *shadowMap);
    void addObj(Obj *object);
    void addLight(ILight *light);

protected:
    std::vector<ILight*> lights;
    std::vector<IShadowMap*> shadowMaps;
    std::vector<Obj*> objs;
};

#endif
