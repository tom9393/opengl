#include "../include/Renderer.h"
#include "../platform/OpenGL/OpenGLShader.h"
#include <memory>

Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

void Renderer::BeginScene(OrthographicCamera &camera)
{
    m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
};

void Renderer::EndScene()
{
};

void Renderer::Init()
{
    RenderCommand::Init();
};

void Renderer::Submit(const std::shared_ptr<VertexArray> &vertexArray, const std::shared_ptr<Shader> &shader, const glm::mat4 &transform)
{
    shader->Bind();
    std::dynamic_pointer_cast<OpenGLShader>(shader)->setMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
    std::dynamic_pointer_cast<OpenGLShader>(shader)->setMat4("u_Transform", transform);
    std::dynamic_pointer_cast<OpenGLShader>(shader)->setInt("u_Texture", 0);

    vertexArray->Bind();
    RenderCommand::DrawIndexed(vertexArray);
};
