#pragma once

#include "../Buffer.h"
#include "Model.h"
#include "RenderCommand.h"
#include "RendererAPI.h"
#include "Shader.h"
#include "../Mesh.h"
#include "../../platform/OpenGL/OpenGLTexture.h"

namespace Engine
{

    struct Statistics
    {
        unsigned int DrawCalls = 0;
        unsigned int VerticeCount = 0;
        unsigned int IndicesCount = 0;
    };

    struct RenderCall
    {
        Shared<Mesh> mesh;
        glm::mat4 transform;
        
        RenderCall(const Shared<Mesh> &mesh, const glm::mat4 &transform) : mesh(mesh), transform(transform) {};
    };

    struct RenderStorage
    {
        ShaderLibrary* shaderLibrary;
        Shared<SSBOBuffer> matrixBuffer;
        Shared<UniformBuffer> uniformBuffer;
        Statistics statistics;
        std::vector<RenderCall> renderQueue;
    };

    struct LineVertex {
        glm::vec3 Position;
        glm::vec4 Color;
    };

    struct LineRendererData {
        static const uint32_t MaxLines = 10000;
        static const uint32_t MaxVertices = MaxLines * 2;

        Shared<VertexArray> LineVertexArray;
        Shared<VertexBuffer> LineVertexBuffer;
        Shared<Shader> LineShader;

        uint32_t LineVertexCount = 0;
        LineVertex* LineVertexBufferBase = nullptr;
        LineVertex* LineVertexBufferPtr = nullptr;
    };

    class Renderer
    {
        public:

        static void Init();
        static void OnWindowResize(unsigned int width, unsigned int height);

        static void ToggleWireframe();

        static void BeginScene(const glm::mat4& viewProjection);
        static void BeginScene();
        static void EndScene();

        static void SetTexture(TextureLayout &textureLayout);
        static void Submit(const Shared<Mesh> &mesh, glm::mat4 &transform);
        static void SubmitModel(const Shared<Model> &model, const glm::mat4 &transform);

        static void DrawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color);

        static RendererAPI::API Get() { return RendererAPI::GetAPI(); }

        static void ResetStatistics()
        {
            memset(&s_Data.statistics, 0, sizeof(Statistics));
        }
        static Statistics &GetStatistics() { return s_Data.statistics; }

        private:
        static void FlushLines();

        private:
        struct SceneData
        {
            glm::mat4 ViewProjectionMatrix;
        };

        static SceneData s_SceneData;
        static RenderStorage s_Data;
        static LineRendererData s_LineData;

    };

};
