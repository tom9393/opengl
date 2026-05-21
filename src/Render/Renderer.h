#pragma once

#include "Buffer.h"
#include "Line.h"
#include "Point.h"
#include "RenderCommand.h"
#include "RendererAPI.h"
#include "RenderContext.h"
#include "Shader.h"
#include "Utils/MathTools.h"

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

    class Mesh;
    class Material;

    class Renderer
    {
        public:

        static void Init();
        static void Shutdown();
        static void OnWindowResize(unsigned int width, unsigned int height);

        static void ToggleWireframe();

        static void BeginScene(const RenderContext &context);
        static void EndScene();

        static void SetMaterial(const Shared<Material> &material);
        static void SubmitMesh(const Shared<Mesh> &mesh, const Shared<Material> &material, const glm::mat4 &transform);
        static void SubmitInstancedMesh(const Shared<Mesh> &mesh, const Shared<Material> &material, unsigned int count);

        static void DrawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color);
        static void DrawDot(const glm::vec3 &position, const glm::vec4 &color, float size = 5.0f);
        static void DrawCircle(const glm::vec3 &center, float radius, const glm::vec4 &color, uint32_t segments = 32);
        static void DrawFullscreenQuad();

        static RendererAPI::API Get() { return RendererAPI::GetAPI(); }

        static void ResetStatistics()
        {
            memset(&s_Data.statistics, 0, sizeof(Statistics));
        }
        static Statistics &GetStatistics() { return s_Data.statistics; }

        private:
        static void FlushLines();
        static void FlushPoints();

        private:
        struct SceneData
        {
            RenderContext context;
            uintptr_t s_LastBoundMaterial;
        };

        static SceneData s_SceneData;
        static RenderStorage s_Data;
        static LineData s_LineData;
        static PointData s_PointData;

    };

};
