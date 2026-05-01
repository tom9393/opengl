#include "Application.h"
#include "EditorLayer.h"
#include "TimeStep.h"
#include "Render/Renderer.h"
#include "KeyCodes.h"
#include "Events/EventBroker.h"
#include "Asset/AssetManager.h"
#include "Asset/AssetUploadQueue.h"

namespace Engine
{

    Application* Application::s_Instance = nullptr;

    Application::Application()
    {
        s_Instance = this;
        m_Window = Window::Create();
        m_LayerStack = new LayerStack();
        m_ThreadPool = new ThreadPool(std::thread::hardware_concurrency());
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
        AssetManager::Init();

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlayer(m_ImGuiLayer);
    };

    Application::~Application()
    {
        EventBroker::Get().UnsubscribeAll(this);

        AssetManager::Shutdown();

        delete m_ThreadPool;
        delete m_LayerStack;
        delete m_Window;
    };

    void Application::Run()
    {
        while (m_Running)
        {
            m_Window->UpdateFrameTime();
            TimeStep ts = m_Window->GetFrameTime();

            AssetUploadQueue::Process();

            if (!m_Minimized)
            {
                for (Layer* layer: *m_LayerStack)
                {
                    layer->OnUpdate(ts);
                }
            }

            m_ImGuiLayer->Begin();
            for (Layer* layer: *m_LayerStack)
            {
                layer->OnImGuiRender();
            }
            m_ImGuiLayer->End();

            m_Window->OnUpdate();
        }
    };

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack->PushLayer(layer);
        layer->OnAttach();
    };
    void Application::PushOverlayer(Layer* layer)
    {
        m_LayerStack->PushOverlay(layer);
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
        if (e.getKeyCode() == KEY_ESCAPE)
        {
            WindowCloseEvent e;
            EventBroker::Get().Publish(e);
        }

        return false;
    };

    Runtime::Runtime() : Application()
    {
        //PushLayer(new Testlayer());
    };

    Editor::Editor() : Application()
    {
        PushLayer(new EditorLayer());
    };

};
