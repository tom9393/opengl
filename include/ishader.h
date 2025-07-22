#ifndef ISHADER
#define ISHADER

#include "shader.h"
#include "Camera.h"

class ISetViewPosition : public Shader
{
public:
    void setView(Camera *camera)
    {
        this->camera = camera;
    };
    void setView() {
        setMat4("view", camera->getViewMatrix());
        setMat4("projection", camera->getProjection());
        setVec3("viewPos", camera->getPosition());
    };
private:
    Camera* camera;
};

#endif
