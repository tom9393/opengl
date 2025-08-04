#pragma once

#include "RenderCommand.h"
#include "RendererAPI.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"

class Renderer
{
    public:

    static void Init();
    static void OnWindowResize(uint32_t width, uint32_t height);

    static void BeginScene(std::shared_ptr<Camera> camera);
    static void EndScene();

    static void SetTexture(const std::shared_ptr<Shader> &shader, const std::shared_ptr<Texture> &texture, const unsigned int index);
    static void Submit(const std::shared_ptr<Shader> &shader, const std::shared_ptr<VertexArray> &vertexArray, const glm::mat4 &transform);

    static RendererAPI::API Get() { return RendererAPI::GetAPI(); }

    private:
    struct SceneData
    {
        glm::mat4 ViewProjectionMatrix;
    };

    static SceneData* m_SceneData;

};
