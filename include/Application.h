#pragma once

#include "../pch.h"

#include "ImGuiLayer.h"
#include "LayerStack.h"
#include "Window.h"
#include "events/ApplicationEvent.h"

class Application
{
    public:
    Application();
    virtual ~Application();

    void run();
    void onEvent(Event &e);
    void pushLayer(Layer* layer);
    void pushOverlayer(Layer* layer);

    static Application& Get() { return *s_Instance; }
    Window& GetWindow() { return *m_Window; }

    private:
    static Application* s_Instance;
    ImGuiLayer* m_ImGuiLayer;
    std::unique_ptr<Window> m_Window;
    LayerStack m_LayerStack;
    bool m_Running = true;

    bool onWindowClose(WindowCloseEvent &e);

    Application* createApplication();
};

