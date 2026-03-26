#pragma once

#include "../pch.h"

#include "ImGuiLayer.h"
#include "LayerStack.h"
#include "Utils/ThreadPool.h"
#include "Window.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyboardEvent.h"
#include "Events/MouseEvent.h"

namespace Engine
{

    class Application
    {
        public:
        Application();
        virtual ~Application();

        void Run();
        void PushLayer(Layer* layer);
        void PushOverlayer(Layer* layer);

        void Close() { m_Running = false; };

        static Application& Get() { return *s_Instance; }
        Window& GetWindow() { return *m_Window; }
        ThreadPool& GetThreadPool() { return *m_ThreadPool; }
        ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

        private:
        static Application* s_Instance;
        ImGuiLayer* m_ImGuiLayer;
        std::unique_ptr<Window> m_Window;
        LayerStack m_LayerStack;
        ThreadPool* m_ThreadPool;
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

};
