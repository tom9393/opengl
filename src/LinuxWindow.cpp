#include "../include/LinuxWindow.h"
#include "../include/events/ApplicationEvent.h"
#include "../include/events/MouseEvent.h"
#include "../include/events/KeyboardEvent.h"
#include <GLFW/glfw3.h>

static bool s_GLFWInitialized = false;

static void GLFWErrorCallback(int error, const char* desc)
{
        std::cout << desc << std::endl;
};

Window* Window::Create(const WindowProps &props)
{
    return new LinuxWindow(props);
};

LinuxWindow::LinuxWindow(const WindowProps &props)
{
    Init(props);
};

LinuxWindow::~LinuxWindow()
{
};

void LinuxWindow::Init(const WindowProps &props)
{
    m_Data.height = props.height;
    m_Data.width = props.width;
    m_Data.title = props.title;

    if (!s_GLFWInitialized)
    {
        int ok = glfwInit();
        if (ok) s_GLFWInitialized = true;
        glfwSetErrorCallback(GLFWErrorCallback);
    }

    m_Window = glfwCreateWindow((int)props.width, (int)props.height, props.title.c_str(), nullptr, nullptr);
    glfwMakeContextCurrent(m_Window);
    glfwSetWindowUserPointer(m_Window, &m_Data);
    gladLoadGL();
    setVsync(true);

    // GLFW callbacks
    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
    {
        WindowData &data = *(WindowData*) glfwGetWindowUserPointer(window);
        data.width = width;
        data.height = height;

        WindowResizeEvent event(width, height);
        data.cb(event);
    });

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
        WindowData &data = *(WindowData*)glfwGetWindowUserPointer(window);
        WindowCloseEvent event;
        data.cb(event);
    });

    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        WindowData &data = *(WindowData*)glfwGetWindowUserPointer(window);

        switch (action)
        {
            case GLFW_PRESS:
                {
                    KeyboardPressEvent event(key, 0);
                    data.cb(event);
                    break;
                }
            case GLFW_RELEASE:
                {
                    KeyboardReleaseEvent event(key, 0);
                    data.cb(event);
                    break;
                }
            case GLFW_REPEAT:
                {
                    KeyboardPressEvent event(key, 0);
                    data.cb(event);
                    break;
                }
        }
    });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
        WindowData &data = *(WindowData*)glfwGetWindowUserPointer(window);

        switch (action)
        {
            case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(button);
                    data.cb(event);
                    break;
                }
            case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(button);
                    data.cb(event);
                    break;
                }
        }
    });

    glfwSetScrollCallback(m_Window, [](GLFWwindow *window, double xoffset, double yoffset) {
        WindowData &data = *(WindowData*)glfwGetWindowUserPointer(window);

        MouseScrolledEvent event((float)xoffset, (float)yoffset);
        data.cb(event);

    });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow *window, double xpos, double ypos) {
        WindowData &data = *(WindowData*)glfwGetWindowUserPointer(window);

        MouseMovedEvent event((float)xpos, (float)ypos);
        data.cb(event);

    });
};

void LinuxWindow::Shutdown()
{
    glfwDestroyWindow(m_Window);
};

void LinuxWindow::onUpdate()
{
    glfwPollEvents();
    glfwSwapBuffers(m_Window);
};

void LinuxWindow::setVsync(bool enable)
{
    if (enable)
        glfwSwapInterval(1);
    else
        glfwSwapInterval(0);

    m_Data.VSync = enable;
};

bool LinuxWindow::isVsync()
{
        return m_Data.VSync;
};

void* LinuxWindow::GetNativeWindow()
{
        return m_Window;
};
