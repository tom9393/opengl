#include "../include/shadowshader.h"

Shadowshader::Shadowshader()
{
}

bool Shadowshader::Init()
{
    if (!Shader::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "/home/tom/Coding/c++/opengl/shaders/shadow.vert")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "/home/tom/Coding/c++/opengl/shaders/shadow.frag")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    return true;
}

void Shadowshader::setup()
{
}
void Shadowshader::draw()
{

    unsigned int offset = 0;

    for (unsigned int i = 0; i < dirLights.size(); i++)
    {
        ShadowMap *map = shadowMaps[i];
        DirLight *light = dirLights[i];
        map->BindForWriting();
        setMat4("lightSpaceMatrix", light->getMatrix());
        for (unsigned int j = 0; j < objs.size(); j++)
        {
            Obj *obj = objs[j];
            obj->draw(*this);
        }
        offset++;
    }
    
    for (unsigned int i = 0; i < spotLights.size(); i++)
    {
        ShadowMap *map = shadowMaps[i + offset];
        SpotLight *light = spotLights[i];
        map->BindForWriting();
        setMat4("lightSpaceMatrix", light->getMatrix());
        for (unsigned int j = 0; j < objs.size(); j++)
        {
            Obj *obj = objs[j];
            obj->draw(*this);
        }
    }
    cleanup();
}
void Shadowshader::cleanup()
{
    // Reset
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 1600, 1600);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Shadowshader::addShadowMap(ShadowMap *shadowMap)
{
    shadowMaps.push_back(shadowMap);
}

void Shadowshader::addDirLight(DirLight *light)
{
    dirLights.push_back(light);
}

void Shadowshader::addSpotLight(SpotLight *light)
{
    spotLights.push_back(light);
}

void Shadowshader::addObj(Obj *obj)
{
    objs.push_back(obj);
}
