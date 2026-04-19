#pragma once

#include "Buffer.h"
#include "../Utils/MathTools.h"
#include "RenderCommand.h"
#include "RendererAPI.h"
#include "Shader.h"
#include "../Asset/Mesh.h"
#include "../../platform/OpenGL/OpenGLTexture.h"
#include "../Scene/Components.h"
#include "Line.h"
#include "RenderContext.h"

namespace Engine
{

    struct Statistics
    {
        unsigned int DrawCalls = 0;
        unsigned int MaterialSwitches = 0;
        unsigned int VerticeCount = 0;
        unsigned int IndicesCount = 0;
    };

    struct RenderStorage
    {
        Statistics statistics;
    };

    class Renderer
    {
        public:

        static void Init();
        static void Shutdown();
        static void OnWindowResize(unsigned int width, unsigned int height);

        static void ToggleWireframe();

        static void BeginScene();
        static void EndScene();

        static void SetMaterial(const Shared<Material> &material);
        static void SubmitMesh(const Shared<Mesh> &mesh, const glm::mat4 &transform);
        static void SubmitInstancedMesh(const Shared<Mesh> &mesh, const Shared<Material> &material, unsigned int count);

        static void DrawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color);
        static void DrawFullscreenQuad();

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
            Frustum cameraFrustum;
            uintptr_t s_LastBoundMaterial;
        };

        static SceneData s_SceneData;
        static RenderStorage s_Data;
        static LineData s_LineData;

    };

};
