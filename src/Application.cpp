#include "../include/Application.h"
#include "../include/Input.h"

Application* Application::s_Instance = nullptr;

Application::Application()
{
    s_Instance = this;
    m_Window = std::unique_ptr<Window>(Window::Create());
    m_Window->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));
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
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (Layer* layer: m_LayerStack)
        {
            layer->OnUpdate();
        }

        m_ImGuiLayer->Begin();
        for (Layer* layer: m_LayerStack)
        {
            layer->OnImGuiRender();
        }
        m_ImGuiLayer->End();

        auto [x, y] = Input::GetMousePosition();
        std::cout << x << std::endl;
        std::cout << y << std::endl;

        m_Window->onUpdate();
    }
};

void Application::onEvent(Event &e)
{
    EventDispatcher dispatcher(e);
    dispatcher.dispatch<WindowCloseEvent>(std::bind(&Application::onWindowClose, this, std::placeholders::_1));

    for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
    {
        (*--it)->OnEvent(e);
        if (e.handled)
        {
            break;
        }
    }

};

bool Application::onWindowClose(WindowCloseEvent &e)
{
    m_Running = false;
    return true;
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
