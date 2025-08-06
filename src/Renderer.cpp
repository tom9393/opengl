#include "../include/Renderer.h"
#include "../platform/OpenGL/OpenGLTexture.h"

Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;
RenderStorage* Renderer::m_Data = new RenderStorage;

void Renderer::BeginScene(std::shared_ptr<Camera> camera)
{
    m_SceneData->ViewProjectionMatrix = camera->GetViewProjectionMatrix();
};

void Renderer::EndScene()
{
};

void Renderer::Init()
{
    RenderCommand::Init();

    m_Data->shaderLibrary = new ShaderLibrary();
    const std::string vert = "/home/tom/Coding/c++/opengl/shaders/default.vert";
    const std::string frag = "/home/tom/Coding/c++/opengl/shaders/default.frag";
    std::vector<std::string> paths = { vert, frag };
    std::shared_ptr<Shader> shader = Shader::Create("Texture");
    shader->AddShaders(paths);
    m_Data->shaderLibrary->Add(shader);
};

void Renderer::OnWindowResize(unsigned int width, unsigned int height)
{
    RenderCommand::SetViewport(0, 0, width, height);
};

void Renderer::SetTexture(const std::shared_ptr<Texture> &texture, const unsigned int index)
{
    std::shared_ptr<Shader> shader = m_Data->shaderLibrary->Get("Texture");
    texture->Bind(index);
    std::string textType = "";
    TextureType txtType = std::dynamic_pointer_cast<OpenGLTexture2D>(texture)->GetType();
    if (txtType == TextureType::Diffuse) textType = "material.diffuse";
    else if (txtType == TextureType::Specular) textType = "material.specular";

    shader->setInt(textType, index);
    shader->setFloat("material.shininess", 0.75f);
};

void Renderer::Submit(const std::shared_ptr<VertexArray> &vertexArray, const glm::mat4 &transform)
{
    std::shared_ptr<Shader> shader = m_Data->shaderLibrary->Get("Texture");
    shader->Bind();
    shader->setMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
    shader->setMat4("u_Transform", transform);

    vertexArray->Bind();
    RenderCommand::DrawIndexed(vertexArray);
    vertexArray->Unbind();
};

void Renderer::ToggleWireframe()
{
    RenderCommand::ToggleWireframe();
};
