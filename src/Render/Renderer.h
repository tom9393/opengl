#pragma once

#include "Buffer.h"
#include "../Utils/MathTools.h"
#include "RenderCommand.h"
#include "RendererAPI.h"
#include "Shader.h"
#include "../Asset/Mesh.h"
#include "../../platform/OpenGL/OpenGLTexture.h"
#include "../Scene/Components.h"
#include "SceneRenderer.h"
#include "Line.h"

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
        Shared<UniformBuffer> CameraUniformBuffer;
        Shared<UniformBuffer> LightUniformBuffer;
        Statistics statistics;
    };

    struct CameraData
    {
        glm::mat4 ViewProjection;
        glm::vec3 CameraPosition;
        float padding;
    };

    struct PointLightData 
    {
        glm::vec4 Position; // w is ignored
        glm::vec4 Color;    // rgb = color * intensity, w is ignored
    };

    struct SceneLightData 
    {
        PointLightData Lights[64]; // Max 64 lights for this example
        int LightCount;
        int padding[3];           // Padding to align
    };

    class Renderer
    {
        public:

        static void Init();
        static void Shutdown();
        static void OnWindowResize(unsigned int width, unsigned int height);

        static void ToggleWireframe();

        static void BeginScene(RenderContext& context);
        static void EndScene();

        static void SetMaterial(const Shared<Material> &material);
        static void SubmitMesh(const Shared<Mesh> &mesh, const glm::mat4 &transform);
        static void SubmitInstancedMesh(const Shared<Mesh> &mesh, const Shared<Material> &material, unsigned int count);

        static void UpdateLightBuffer(SceneLightData &data);

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
            Frustum cameraFrustum;
            uintptr_t s_LastBoundMaterial;
        };

        static SceneData s_SceneData;
        static RenderStorage s_Data;
        static LineData s_LineData;

    };

};
