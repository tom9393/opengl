#include "../include/Application.h"
#include "../include/Input.h"
#include "../include/BufferLayout.h"
#include "../include/Renderer.h"

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

    unsigned int indices[3] = { 0, 1, 2 };

    m_VertexArray.reset(VertexArray::Create());


    std::shared_ptr<VertexBuffer> vertexBuffer = std::shared_ptr<VertexBuffer>(VertexBuffer::Create(vertices, sizeof(vertices)));
    std::shared_ptr<IndexBuffer> indexBuffer = std::shared_ptr<IndexBuffer>(IndexBuffer::Create(indices, sizeof(uint32_t)));

    BufferLayout layout = {
        { ShaderDataType::Float3, "a_Position" }
    };

    vertexBuffer->SetLayout(layout);

    m_VertexArray->AddVertexBuffer(vertexBuffer);
    m_VertexArray->SetIndexBuffer(indexBuffer);
};

Application::~Application()
{
};

void Application::run()
{

    while (m_Running)
    {
        RenderCommand::SetClearColor({0.2f, 0.2f, 0.2f, 1.0f});
        RenderCommand::Clear();

        Renderer::BeginScene();
        Renderer::Submit(m_VertexArray);

        Renderer::EndScene();

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
