#include "Renderer.h"
#include "Asset/Mesh.h"
#include "Asset/Material.h"
#include "Buffer.h"
#include "EnvironmentMap.h"
#include "OpenGL/OpenGLShader.h"
#include "Utils/MathTools.h"

namespace Engine
{

    Renderer::SceneData Renderer::s_SceneData;
    RenderStorage Renderer::s_Data;
    LineData Renderer::s_LineData;
    PointData Renderer::s_PointData;

    void Renderer::BeginScene(const RenderContext &context)
    {
        s_SceneData.context = context;
        s_SceneData.s_LastBoundMaterial = 0;
    };

    void Renderer::EndScene()
    {
        FlushLines();
        FlushPoints();
    };

    void Renderer::Init()
    {
        PROFILE_FUNCTION();

        RenderCommand::Init();
        ShaderLibrary::Init();

        // Build the env-independent BRDF integration LUT and fallback cubemap once now while no graph framebuffer is bound
        EnvironmentMap::GetBRDFLUT();
        EnvironmentMap::GetFallbackCubemap();

        s_LineData.Init();
        s_PointData.Init();

        switch (Renderer::Get())
        {
            case RendererAPI::API::None:
                break;
            case RendererAPI::API::OpenGL:
                break;
            case RendererAPI::API::Vulkan:
                break;
            default:
                break;
        }
    }

    void Renderer::Shutdown()
    {
        s_LineData.Shutdown();
        s_PointData.Shutdown();
        ShaderLibrary::Shutdown();
    }

    void Renderer::OnWindowResize(unsigned int width, unsigned int height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    };

    void Renderer::SetMaterial(const Shared<Material> &material)
    {
        PROFILE_FUNCTION();

        if (!material) return;
        material->Bind();
    }

    void Renderer::FlushLines()
    {
        if (s_LineData.VertexCount == 0) return;

        s_LineData.LineVertexBuffer->SetData(s_LineData.VertexBufferBase, s_LineData.GetDataSize());

        s_LineData.LineShader->Bind();
        s_LineData.LineShader->SetMat4("u_ViewProjection", s_SceneData.context.ViewProjection);

        RenderCommand::DrawLines(s_LineData.LineVertexArray, s_LineData.VertexCount);

        s_LineData.ResetBatch();
        s_Data.statistics.DrawCalls++;
    }

    void Renderer::FlushPoints()
    {
        if (s_PointData.VertexCount == 0) return;

        s_PointData.PointVertexBuffer->SetData(s_PointData.VertexBufferBase, s_PointData.GetDataSize());

        s_PointData.PointShader->Bind();
        s_PointData.PointShader->SetMat4("u_ViewProjection", s_SceneData.context.ViewProjection);

        RenderCommand::DrawPoints(s_PointData.PointVertexArray, s_PointData.VertexCount);

        s_PointData.ResetBatch();
        s_Data.statistics.DrawCalls++;
    }

    void Renderer::DrawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color)
    {
        if (s_LineData.IsFull()) {
            FlushLines();
        }

        s_LineData.AddVertex(p0, color);
        s_LineData.AddVertex(p1, color);
    }

    void Renderer::DrawDot(const glm::vec3 &position, const glm::vec4 &color, float size)
    {
        if (s_PointData.IsFull()) {
            FlushPoints();
        }

        s_PointData.AddVertex(position, color, size);
    }

    void Renderer::DrawCircle(const glm::vec3 &center, float radius, const glm::vec4 &color, uint32_t segments)
    {
        // Screen-facing: derive world-space screen right/up from the view-projection so the circle outline always lies parallel to the camera plane
        glm::mat4 invViewProj = glm::inverse(s_SceneData.context.ViewProjection);
        auto unproject = [&](float x, float y) {
            glm::vec4 p = invViewProj * glm::vec4(x, y, 0.0f, 1.0f);
            return glm::vec3(p) / p.w;
        };
        glm::vec3 origin = unproject(0.0f, 0.0f);
        glm::vec3 right = glm::normalize(unproject(1.0f, 0.0f) - origin);
        glm::vec3 up = glm::normalize(unproject(0.0f, 1.0f) - origin);

        const float step = 6.28318530718f / (float)segments;
        glm::vec3 prev = center + right * radius;
        for (uint32_t i = 1; i <= segments; i++) {
            float angle = step * (float)i;
            glm::vec3 curr = center + (right * cosf(angle) + up * sinf(angle)) * radius;
            DrawLine(prev, curr, color);
            prev = curr;
        }
    }

    void Renderer::DrawFullscreenQuad()
    {
        RenderCommand::DrawFullscreenQuad();
    }

    void Renderer::SubmitMesh(const Shared<Mesh> &mesh, const Shared<Material> &material, const glm::mat4 &transform)
    {
        if (!mesh->GetVAO()) return;

        uintptr_t currentMaterialKey = reinterpret_cast<uintptr_t>(material.get());
        if (currentMaterialKey != s_SceneData.s_LastBoundMaterial) {
            SetMaterial(material);
            
            s_SceneData.s_LastBoundMaterial = currentMaterialKey;
            s_Data.statistics.MaterialSwitches++;
        }

        //s_Data.shaderLibrary->Get("Texture")->SetMat4("u_Transform", transform);

        Shared<VertexArray> VAO = mesh->GetVAO();
        VAO->Bind();
        RenderCommand::DrawIndexed(VAO);
        s_Data.statistics.DrawCalls++;
        s_Data.statistics.VerticeCount += mesh->GetVertices().size();
        s_Data.statistics.IndicesCount += mesh->GetIndices().size();
    }

    void Renderer::SubmitInstancedMesh(const Shared<Mesh> &mesh, const Shared<Material> &material, unsigned int count)
    {
        if (!mesh->GetVAO()) return;

        uintptr_t currentMaterialKey = reinterpret_cast<uintptr_t>(material.get());
        if (currentMaterialKey != s_SceneData.s_LastBoundMaterial) {
            SetMaterial(material);
            
            s_SceneData.s_LastBoundMaterial = currentMaterialKey;
            s_Data.statistics.MaterialSwitches++;
        }

        Shared<VertexArray> VAO = mesh->GetVAO();
        VAO->Bind();
        RenderCommand::DrawIndexedInstances(VAO, count);
        s_Data.statistics.DrawCalls++;
        s_Data.statistics.VerticeCount += mesh->GetVertices().size();
        s_Data.statistics.IndicesCount += mesh->GetIndices().size();
    }

    void Renderer::ToggleWireframe()
    {
        RenderCommand::ToggleWireframe();
    };

};
