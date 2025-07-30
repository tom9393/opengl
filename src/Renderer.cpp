#include "../include/Renderer.h"

Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

void Renderer::BeginScene(OrthographicCamera &camera)
{
    m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
};

void Renderer::EndScene()
{
};

void Renderer::Submit(const std::shared_ptr<VertexArray> &vertexArray, Shader &shader, const glm::mat4 &transform)
{
    shader.Enable();
    shader.setMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
    shader.setMat4("u_Transform", transform);
    vertexArray->Bind();
    RenderCommand::DrawIndexed(vertexArray);
};
