#include "../include/Application.h"
#include "../include/TestLayer.h"
#include "../include/EditorLayer.h"
#include "../include/TimeStep.h"
#include "../include/Renderer.h"
#include "../include/KeyCodes.h"
#include "../include/Events/EventBroker.h"

namespace Engine
{

    Application* Application::s_Instance = nullptr;

    Application::Application()
    {
        s_Instance = this;
        m_Window = std::unique_ptr<Window>(Window::Create());
        m_Window->SetEventCallback(
            [](Engine::Event& e)
            {
                EventBroker::Get().Publish(e); 
            }
        );

        EventBroker::Get().Subscribe<WindowCloseEvent>(BIND_FN(OnWindowClose), this);
        EventBroker::Get().Subscribe<WindowResizeEvent>(BIND_FN(OnWindowResize), this);
        EventBroker::Get().Subscribe<MouseButtonPressedEvent>(BIND_FN(OnMouseClick), this);
        EventBroker::Get().Subscribe<KeyboardPressEvent>(BIND_FN(OnKeyPress), this);

        Renderer::Init();

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlayer(m_ImGuiLayer);
    };

    Application::~Application()
    {
        EventBroker::Get().UnsubscribeAll(this);
    };

    void Application::Run()
    {

        while (m_Running)
        {
            float time = (float)glfwGetTime();
            TimeStep ts = time - m_LastTime;
            m_LastTime = time;

            if (!m_Minimized)
            {
                for (Layer* layer: m_LayerStack)
                {
                    layer->OnUpdate(ts);
                }
            }

            m_ImGuiLayer->Begin();
            for (Layer* layer: m_LayerStack)
            {
                layer->OnImGuiRender();
            }
            m_ImGuiLayer->End();

            m_Window->OnUpdate();
        }
    };

    void Application::OnEvent(Event &e)
    {
        return;
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_FN(OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_FN(OnWindowResize));
        dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_FN(OnMouseClick));
        dispatcher.Dispatch<KeyboardPressEvent>(BIND_FN(OnKeyPress));

        // Leave this here for now
        // for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
        // {
        //     (*--it)->OnEvent(e);
        //     if (e.handled)
        //     {
        //         break;
        //     }
        // }
    };

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    };
    void Application::PushOverlayer(Layer* layer)
    {
        m_LayerStack.PushOverlay(layer);
        layer->OnAttach();
    };

    bool Application::OnWindowClose(WindowCloseEvent &e)
    {
        m_Running = false;
        return true;
    };

    bool Application::OnWindowResize(WindowResizeEvent &e)
    {
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            m_Minimized = true;
            return false;
        }
        m_Minimized = false;
        Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

        return false;
    };

    bool Application::OnMouseClick(MouseButtonPressedEvent &e)
    {
        return false;
    };

    bool Application::OnKeyPress(KeyboardPressEvent &e)
    {
        if (e.getKeyCode() == KEY_Q)
        {
            WindowCloseEvent e;
            EventBroker::Get().Publish(e);
        }

        if (e.getKeyCode() == KEY_E)
        {
            Renderer::ToggleWireframe();
        }

        return false;
    };

    Runtime::Runtime() : Application()
    {
        PushLayer(new Testlayer());
    };

    Editor::Editor() : Application()
    {
        PushLayer(new EditorLayer());
    };

};
