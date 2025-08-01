#include "../pch.h"
#include "../include/testlayer.h"
#include "../include/Renderer.h"
#include "../platform/OpenGL/OpenGLShader.h"

//Testlayer::Testlayer() : m_CameraController(std::make_shared<OrthographicCamera>(), 2.0f)
Testlayer::Testlayer() : m_CameraController(std::make_shared<PerspectiveCamera>(), 2.0f)
{

    float vertices[5*4] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f
    };

    std::shared_ptr<Shader> shad = Shader::Create("Texture");

    const std::string vert = "/home/tom/Coding/c++/opengl/shaders/tmp.vert";
    const std::string frag = "/home/tom/Coding/c++/opengl/shaders/tmp.frag";
    std::dynamic_pointer_cast<OpenGLShader>(shad)->AddShader(vert);
    std::dynamic_pointer_cast<OpenGLShader>(shad)->AddShader(frag);
    std::dynamic_pointer_cast<OpenGLShader>(shad)->Finalize();

    m_ShaderLibrary.Add(shad);

    unsigned int indices[6] = { 0, 1, 2, 2, 3, 0};

    m_VertexArray.reset(VertexArray::Create());
    m_Texture = Texture2D::Create("/home/tom/Coding/c++/opengl/assets/Board.jpg");

    std::shared_ptr<VertexBuffer> vertexBuffer = std::shared_ptr<VertexBuffer>(VertexBuffer::Create(vertices, sizeof(vertices)));
    std::shared_ptr<IndexBuffer> indexBuffer = std::shared_ptr<IndexBuffer>(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

    BufferLayout layout = {
        { ShaderDataType::Float3, "a_Position" },
        { ShaderDataType::Float2, "a_TexCoord" }
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

void Testlayer::OnUpdate(TimeStep ts)
{

    m_CameraController.OnUpdate(ts);

    RenderCommand::SetClearColor({0.2f, 0.2f, 0.2f, 1.0f});
    RenderCommand::Clear();

    Renderer::BeginScene(m_CameraController.GetCamera());

    glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
    m_Texture->Bind(0);
    Renderer::Submit(m_VertexArray, m_ShaderLibrary.Get("Texture"), pos);

    Renderer::EndScene();
};

void Testlayer::OnImGuiRender()
{
};

void Testlayer::OnEvent(Event& e)
{
    m_CameraController.OnEvent(e);
};
