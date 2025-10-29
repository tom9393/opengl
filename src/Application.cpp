#include "../include/Application.h"
#include "../include/TestLayer.h"
#include "../include/EditorLayer.h"
#include "../include/TimeStep.h"
#include "../include/Renderer.h"
#include "../include/Input.h"
#include "../include/KeyCodes.h"
#include "../include/MouseButtonCodes.h"

Application* Application::s_Instance = nullptr;

Application::Application()
{
    s_Instance = this;
    m_Window = std::unique_ptr<Window>(Window::Create());
    m_Window->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));

    Renderer::Init();

    m_ImGuiLayer = new ImGuiLayer();
    pushOverlayer(m_ImGuiLayer);
};

Application::~Application()
{
};

void Application::run()
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

        m_Window->onUpdate();
    }
};

void Application::onEvent(Event &e)
{
    EventDispatcher dispatcher(e);
    dispatcher.dispatch<WindowCloseEvent>(std::bind(&Application::OnWindowClose, this, std::placeholders::_1));
    dispatcher.dispatch<WindowResizeEvent>(std::bind(&Application::OnWindowResize, this, std::placeholders::_1));
    dispatcher.dispatch<MouseButtonPressedEvent>(std::bind(&Application::OnMouseClick, this, std::placeholders::_1));
    dispatcher.dispatch<KeyboardPressEvent>(std::bind(&Application::OnKeyPress, this, std::placeholders::_1));

    for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
    {
        (*--it)->OnEvent(e);
        if (e.handled)
        {
            break;
        }
    }

    if (Input::IsKeyPressed(KEY_Q))
    {
        m_Running = false;
    }
};

void Application::pushLayer(Layer* layer)
{
    m_LayerStack.PushLayer(layer);
    layer->OnAttach();
};
void Application::pushOverlayer(Layer* layer)
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
    if (e.GetMouseButton() == MOUSE_BUTTON_LEFT)
    {
    }

    // if (e.GetMouseButton() == MOUSE_BUTTON_RIGHT)
    // {
    //     m_Window->ToggleCursor();
    // }

    return false;
};

bool Application::OnKeyPress(KeyboardPressEvent &e)
{
    if (e.getKeyCode() == KEY_Q)
    {
        WindowCloseEvent e;
        onEvent(e);
    }

    if (e.getKeyCode() == KEY_E)
    {
        Renderer::ToggleWireframe();
    }

    return false;
};

Runtime::Runtime() : Application()
{
    pushLayer(new Testlayer());
};

Editor::Editor() : Application()
{
    pushLayer(new EditorLayer());
};
