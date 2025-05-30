#ifndef SHADOWSHADER
#define SHADOWSHADER

#include "light.h"
#include "obj.h"
#include "shader.h"
#include "shadowmap.h"
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

class Shadowshader : public Shader
{
public:

    Shadowshader();
    virtual bool Init();

    void setup();
    void draw();
    void cleanup();

    void addShadowMap(ShadowMap *shadowMap);
    void addSpotLight(SpotLight *light);
    void addDirLight(DirLight *light);
    void addObj(Obj *light);

private:
    std::vector<ShadowMap*> shadowMaps;
    std::vector<SpotLight*> spotLights;
    std::vector<DirLight*> dirLights;
    std::vector<PointLight*> pointLights;
    std::vector<Obj*> objs;
};


#endif
