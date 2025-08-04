#include "../pch.h"
#include "../include/MouseButtonCodes.h"
#include "../include/testlayer.h"
#include "../include/Renderer.h"
#include "../platform/OpenGL/OpenGLShader.h"

Testlayer::Testlayer() : m_CameraController(std::make_shared<Camera>(), 2.0f)
{
    std::shared_ptr<Shader> shad = Shader::Create("Texture");

    const std::string vert = "/home/tom/Coding/c++/opengl/shaders/default.vert";
    const std::string frag = "/home/tom/Coding/c++/opengl/shaders/default.frag";
    std::dynamic_pointer_cast<OpenGLShader>(shad)->AddShader(vert);
    std::dynamic_pointer_cast<OpenGLShader>(shad)->AddShader(frag);
    std::dynamic_pointer_cast<OpenGLShader>(shad)->Finalize();

    m_ShaderLibrary.Add(shad);

    stbi_set_flip_vertically_on_load(true);  
    m_Model = std::make_shared<Model>("/home/tom/Coding/c++/opengl/assets/Leopard2A4.assbin");
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

    RenderCommand::SetClearColor({0.2f, 0.2f, 0.2f, 1.0f});
    RenderCommand::Clear();

    if (m_CameraActive)
    {
        m_CameraController.OnUpdate(ts);
        Renderer::BeginScene(m_CameraController.GetCamera());
    }

    std::vector<std::shared_ptr<Mesh>> meshes = m_Model->GetMeshes();
    std::shared_ptr<Shader> shader = m_ShaderLibrary.Get("Texture");

    glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        std::vector<std::shared_ptr<Texture2D>> textures = meshes[i]->GetTextures();
        for (auto& texture : textures)
        {
            Renderer::SetTexture(shader, texture, i);
        }
        Renderer::Submit(shader, meshes[i]->GetVao(), pos);
    }

    Renderer::EndScene();
};

void Testlayer::OnImGuiRender()
{
};

void Testlayer::OnEvent(Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.dispatch<MouseButtonPressedEvent>(std::bind(&Testlayer::OnMouseButtonPressed, this, std::placeholders::_1));
    m_CameraController.OnEvent(e);
};

bool Testlayer::OnMouseButtonPressed(MouseButtonPressedEvent &e)
{
    if (e.GetMouseButton() == MOUSE_BUTTON_LEFT)
    {
        m_CameraActive = false;
    }

    if (e.GetMouseButton() == MOUSE_BUTTON_RIGHT)
    {
        m_CameraController.GetCamera()->SetFirstMouse(true);
        m_CameraActive = true;
    }

    return false;
};
