#include "../../src/Events/ApplicationEvent.h"
#include "../../src/Events/MouseEvent.h"
#include "../../src/Events/KeyboardEvent.h"
#include "../Asset/DefaultTextures.h"
#include "LinuxWindow.h"
#include "../OpenGL/OpenGLContext.h"
#include <GLFW/glfw3.h>

namespace Engine
{

    void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam)
    {
        // Ignore non-significant error/warning codes
        if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

        std::cout << "---------------" << std::endl;
        std::cout << "Debug message (" << id << "): " <<  message << std::endl;

        switch (severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
            case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
            case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
            case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
        } 
        std::cout << std::endl;
    }

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
        Shutdown();
    };

    void LinuxWindow::Init(const WindowProps &props)
    {
        m_Data.height = props.height;
        m_Data.width = props.width;
        m_Data.title = props.title;
        m_Data.cursor = true;

        if (!s_GLFWInitialized)
        {
            int ok = glfwInit();
            if (ok) s_GLFWInitialized = true;
            glfwSetErrorCallback(GLFWErrorCallback);
        }

        bool debug = true;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        if (debug)
        {
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        }
        m_Window = glfwCreateWindow((int)props.width, (int)props.height, props.title.c_str(), nullptr, nullptr);
        m_Context = new OpenGLContext(m_Window);
        m_Context->Init();

        glfwMakeContextCurrent(m_Window);
        glfwSetWindowUserPointer(m_Window, &m_Data);
        gladLoadGL();

        glEnable(GL_DEPTH_TEST);

        SetVsync(true);
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        if (debug)
        {
            int flags; 
            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
            if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
            {
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Ensures errors are printed as they happen
                glDebugMessageCallback(glDebugOutput, nullptr);
                unsigned int unusedIds[] = { 131140 };
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, unusedIds, GL_TRUE);
            }
        }

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
        DefaultTextures::Shutdown();
        glfwDestroyWindow(m_Window);
    };

    void LinuxWindow::OnUpdate()
    {
        PROFILE_FUNCTION();
        glfwPollEvents();
        m_Context->SwapBuffers();
    };

    void LinuxWindow::SetVsync(bool enable)
    {
        if (enable)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);

        m_Data.VSync = enable;
    };

    bool LinuxWindow::IsVsync()
    {
        return m_Data.VSync;
    };

    void LinuxWindow::UpdateFrameTime()
    {
        float current = (float) glfwGetTime();
        m_CurrentFrame = current - m_PreviousFrame;
        m_PreviousFrame = current;
    }

    float LinuxWindow::GetFrameTime()
    {
        return m_CurrentFrame;
    }

    void* LinuxWindow::GetNativeWindow()
    {
        return m_Window;
    };

    void LinuxWindow::EnableCursor()
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_Data.cursor = true;
    }

    void LinuxWindow::DisableCursor()
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_Data.cursor = false;
    }

    bool LinuxWindow::IsCursor()
    {
        return m_Data.cursor;
    };

};
