#pragma once

#include "../pch.h"

#include "Window.h"
#include "events/ApplicationEvent.h"

class Application
{
    public:
    Application();
    virtual ~Application();

    void run();

    void onEvent(Event &e);

    private:
    std::unique_ptr<Window> m_Window;
    bool m_Running = true;

    bool onWindowClose(WindowCloseEvent &e);

    Application* createApplication();
};

