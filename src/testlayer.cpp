#include "../include/testlayer.h"
#include "../include/Renderer.h"
#include "../include/KeyCodes.h"
#include "../include/Input.h"

Testlayer::Testlayer() : m_Camera(-3.0f, 3.0f, -3.0f, 3.0f), m_CameraPosition(glm::vec3(0.0f)), m_Shader(Shader::Create("/home/tom/Coding/c++/opengl/shaders/tmp.vert", "/home/tom/Coding/c++/opengl/shaders/tmp.frag"))
{
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

Testlayer::~Testlayer()
{
};

void Testlayer::OnAttach() {
    std::cout << "Testlayer attached" << std::endl;
};

void Testlayer::OnDetach() {
    std::cout << "Testlayer detached" << std::endl;
};

void Testlayer::OnUpdate(TimeStep ts) {

    if (Input::IsKeyPressed(KEY_LEFT))
        m_CameraPosition.x -= m_CameraSpeed * ts;
    if (Input::IsKeyPressed(KEY_RIGHT))
        m_CameraPosition.x += m_CameraSpeed * ts;
    if (Input::IsKeyPressed(KEY_DOWN))
        m_CameraPosition.y -= m_CameraSpeed * ts;
    if (Input::IsKeyPressed(KEY_UP))
        m_CameraPosition.y += m_CameraSpeed * ts;

    RenderCommand::SetClearColor({0.2f, 0.2f, 0.2f, 1.0f});
    RenderCommand::Clear();

    m_Camera.SetPosition(m_CameraPosition);
    m_Camera.SetRotation(0.0f);

    Renderer::BeginScene(m_Camera);

    glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
    Renderer::Submit(m_VertexArray, m_Shader, pos);

    Renderer::EndScene();
};

void Testlayer::OnImGuiRender() {
};

void Testlayer::OnEvent(Event& event) {
};
