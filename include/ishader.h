#ifndef ISHADER
#define ISHADER

#include "Camera.h"
#include "light.h"

class ISetViewPosition : public Shader
{
public:
    void setView(IView *view)
    {
        this->view = view;
    };
    void setView() {
        setMat4("view", view->getViewMatrix());
        setMat4("projection", view->getProjection());
        setVec3("viewPos", view->getPosition());
    };
private:
    IView* view;
};

#endif
