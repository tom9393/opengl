#pragma once

#include "OrthographicCamera.h"
#include "RenderCommand.h"
#include "RendererAPI.h"
#include "shader.h"

class Renderer
{
    public:

    static void BeginScene(OrthographicCamera &camera);
    static void EndScene();

    static void Submit(const std::shared_ptr<VertexArray> &vertexArray, Shader &shader);

    static RendererAPI::API Get() { return RendererAPI::GetAPI(); }

    private:
    struct SceneData
    {
        glm::mat4 ViewProjectionMatrix;
    };

    static SceneData* m_SceneData;

};
