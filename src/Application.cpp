#include "../include/Application.h"

Application::Application()
{
    m_Window = std::unique_ptr<Window>(Window::Create());
    m_Window->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));
};

Application::~Application()
{
};

void Application::run()
{

    while (m_Running)
    {
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        m_Window->onUpdate();
    }
};

void Application::onEvent(Event &e)
{
    EventDispatcher dispatcher(e);
    dispatcher.dispatch<WindowCloseEvent>(std::bind(&Application::onWindowClose, this, std::placeholders::_1));
};

bool Application::onWindowClose(WindowCloseEvent &e)
{
    m_Running = false;
    return true;
};
