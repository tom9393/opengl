#pragma once

#include "OrthographicCamera.h"
#include "RenderCommand.h"
#include "RendererAPI.h"
#include "Shader.h"

class Renderer
{
    public:

    static void Init();

    static void BeginScene(OrthographicCamera &camera);
    static void EndScene();

    static void Submit(const std::shared_ptr<VertexArray> &vertexArray, const std::shared_ptr<Shader> &shader, const glm::mat4 &transform);

    static RendererAPI::API Get() { return RendererAPI::GetAPI(); }

    private:
    struct SceneData
    {
        glm::mat4 ViewProjectionMatrix;
    };

    static SceneData* m_SceneData;

};
