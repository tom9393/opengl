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

    void addDirLight(DirLight *light);
    void addPointLight(PointLight *light);
    void addSpotLight(SpotLight *light);
    void addShadowMap(ShadowMap &shadowMap);
    void addDirMap(ShadowMap &shadowMap);
    void addObj(Obj &object);

private:
    std::vector<DirLight*> dirLights;
    std::vector<PointLight*> pointLights;
    std::vector<SpotLight*> spotLights;
    std::vector<ShadowMap> shadowMaps;
    std::vector<ShadowMap> dirMaps;
    std::vector<Obj> objs;
};

#endif
