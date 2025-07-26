#include "../include/Application.h"
#include "../include/Input.h"
#include <GL/gl.h>

Application* Application::s_Instance = nullptr;

Application::Application()
{
    s_Instance = this;
    m_Window = std::unique_ptr<Window>(Window::Create());
    m_Window->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));
    m_ImGuiLayer = new ImGuiLayer();
    pushOverlayer(m_ImGuiLayer);

    float vertices[3*3] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f
    };

    m_VertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    unsigned int indices[3] = { 0, 1, 2 };
    m_IndexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
};

Application::~Application()
{
};

void Application::run()
{

    while (m_Running)
    {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_VertexBuffer->Bind();
        glDrawElements(GL_TRIANGLES, m_IndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);

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
