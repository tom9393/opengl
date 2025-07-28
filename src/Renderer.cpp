#include "../include/Renderer.h"

Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

void Renderer::BeginScene(OrthographicCamera &camera)
{
    m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
};

void Renderer::EndScene()
{
};

void Renderer::Submit(const std::shared_ptr<VertexArray> &vertexArray, Shader &shader)
{
    shader.Enable();
    shader.setMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
    vertexArray->Bind();
    RenderCommand::DrawIndexed(vertexArray);
};
