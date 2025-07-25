#include "../pch.h"
#include "../include/OpenGLContext.h"

OpenGLContext::OpenGLContext(GLFWwindow* window) : m_WindowHandle(window)
{
};

void OpenGLContext::Init()
{
    glfwMakeContextCurrent(m_WindowHandle);
    gladLoadGL();
};

void OpenGLContext::SwapBuffers()
{
    glfwSwapBuffers(m_WindowHandle);
};
