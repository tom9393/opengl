#pragma once

#include "Batch.h"
#include "Buffer.h"
#include "RenderCommand.h"
#include "RendererAPI.h"
#include "Shader.h"
#include "Camera.h"
#include "mesh.h"
#include "../platform/OpenGL/OpenGLTexture.h"

struct Statistics
{
    unsigned int DrawCalls = 0;
    unsigned int VerticeCount = 0;
    unsigned int IndicesCount = 0;
};

struct RenderStorage
{
    ShaderLibrary* shaderLibrary;
    std::shared_ptr<SSBOBuffer> matrixBuffer;
    Batch* batch = nullptr;
    Statistics statistics;
};

class Renderer
{
    public:

    static void Init();
    static void OnWindowResize(unsigned int width, unsigned int height);

    static void ToggleWireframe();

    static void BeginScene(std::shared_ptr<Camera> camera);
    static void EndScene();

    static void SetTexture(const std::shared_ptr<Mesh> &mesh);
    static void Submit(const std::shared_ptr<Mesh> &mesh);

    static RendererAPI::API Get() { return RendererAPI::GetAPI(); }

    static void ResetStatistics()
    {
        memset(&m_Data.statistics, 0, sizeof(Statistics));
    }
    static Statistics &GetStatistics() { return m_Data.statistics; }

    private:
    struct SceneData
    {
        glm::mat4 ViewProjectionMatrix;
    };

    static SceneData m_SceneData;
    static RenderStorage m_Data;

};
