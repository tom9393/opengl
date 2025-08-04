#include "../include/Renderer.h"
#include "../platform/OpenGL/OpenGLShader.h"
#include "../platform/OpenGL/OpenGLTexture.h"

Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

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
};

void Renderer::OnWindowResize(unsigned int width, unsigned int height)
{
    RenderCommand::SetViewport(0, 0, width, height);
};

void Renderer::SetTexture(const std::shared_ptr<Shader> &shader, const std::shared_ptr<Texture> &texture, const unsigned int index)
{
    texture->Bind(index);
    std::string textType = "";
    TextureType txtType = std::dynamic_pointer_cast<OpenGLTexture2D>(texture)->GetType();
    if (txtType == TextureType::Diffuse) textType = "material.diffuse";
    else if (txtType == TextureType::Specular) textType = "material.specular";

    std::dynamic_pointer_cast<OpenGLShader>(shader)->setInt(textType, index);
    std::dynamic_pointer_cast<OpenGLShader>(shader)->setFloat("material.shininess", 0.75f);
};

void Renderer::Submit(const std::shared_ptr<Shader> &shader, const std::shared_ptr<VertexArray> &vertexArray, const glm::mat4 &transform)
{
    shader->Bind();
    std::dynamic_pointer_cast<OpenGLShader>(shader)->setMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
    std::dynamic_pointer_cast<OpenGLShader>(shader)->setMat4("u_Transform", transform);

    vertexArray->Bind();
    RenderCommand::DrawIndexed(vertexArray);
    vertexArray->Unbind();
};

void Renderer::ToggleWireframe()
{
    RenderCommand::ToggleWireframe();
};
