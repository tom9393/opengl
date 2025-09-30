#pragma once

#include "../pch.h"

#include "ImGuiLayer.h"
#include "LayerStack.h"
#include "Window.h"
#include "Events/ApplicationEvent.h"
#include "Events/Event.h"
#include "Events/KeyboardEvent.h"
#include "Events/MouseEvent.h"

class Application
{
    public:
    Application();
    virtual ~Application();

    void run();
    void onEvent(Event &e);
    void pushLayer(Layer* layer);
    void pushOverlayer(Layer* layer);

    void close() { m_Running = false; };

    static Application& Get() { return *s_Instance; }
    Window& GetWindow() { return *m_Window; }
    ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

    private:
    static Application* s_Instance;
    ImGuiLayer* m_ImGuiLayer;
    std::unique_ptr<Window> m_Window;
    LayerStack m_LayerStack;
    bool m_Running = true;
    bool m_Minimized = false;
    float m_LastTime = 0.0f;

    bool OnWindowClose(WindowCloseEvent &e);
    bool OnWindowResize(WindowResizeEvent &e);
    bool OnMouseClick(MouseButtonPressedEvent &e);
    bool OnKeyPress(KeyboardPressEvent &e);
};

class Editor : public Application
{
    public:
    Editor();
};

class Runtime : public Application
{
    public:
    Runtime();
};

