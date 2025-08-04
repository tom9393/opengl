#include "../pch.h"
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

    m_Model = std::make_shared<Model>("/home/tom/Coding/c++/opengl/assets/sukhoi.assbin");
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

    std::vector<Mesh> meshes = m_Model->GetMeshes();

    std::shared_ptr<Shader> shader = m_ShaderLibrary.Get("Texture");


    glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
    for (auto &mesh : meshes)
    {
         std::vector<std::shared_ptr<Texture2D>> textures = mesh.GetTextures();
        unsigned int idx = 0;
        for (auto &texture : textures)
        {
            Renderer::SetTexture(shader, texture, idx);
            idx++;
        }
        Renderer::Submit(shader, mesh.GetVao(), pos);
    }

    Renderer::EndScene();
};

void Testlayer::OnImGuiRender()
{
};

void Testlayer::OnEvent(Event& e)
{
    m_CameraController.OnEvent(e);
};
