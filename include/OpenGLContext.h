#pragma once

#include "Context.h"

class OpenGLContext : public Context
{
    public:
    OpenGLContext(GLFWwindow* window);
    void Init() override;
    void SwapBuffers() override;

    private:
    GLFWwindow* m_WindowHandle;
};
