#include "../../pch.h"
#include "OpenGLContext.h"

namespace Engine
{

    OpenGLContext::OpenGLContext(GLFWwindow* window) : m_WindowHandle(window)
    {
    };

    void OpenGLContext::Init()
    {
        glfwMakeContextCurrent(m_WindowHandle);
        gladLoadGL();
        std::cout << glGetString(GL_RENDERER) << std::endl;
        std::cout << glGetString(GL_VENDOR) << std::endl;
        std::cout << glGetString(GL_VERSION) << std::endl;
    };

    void OpenGLContext::SwapBuffers()
    {
        glfwSwapBuffers(m_WindowHandle);
    };

};
