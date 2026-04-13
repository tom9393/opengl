#include "Renderer.h"
#include "../platform/OpenGL/OpenGLShader.h"
#include "Buffer.h"
#include "../Utils/MathTools.h"

namespace Engine
{

    Renderer::SceneData Renderer::s_SceneData;
    RenderStorage Renderer::s_Data;
    LineData Renderer::s_LineData;

    void Renderer::BeginScene(RenderContext &context)
    {
        CameraData data;
        data.ViewProjection = context.ViewProjection;
        data.CameraPosition = context.CameraPosition;

        s_Data.CameraUniformBuffer->Update(0, sizeof(CameraData), &data);

        s_SceneData.ViewProjectionMatrix = data.ViewProjection;
	    s_SceneData.cameraFrustum = CreateFrustumFromMatrix(data.ViewProjection);
        s_SceneData.s_LastBoundMaterial = 0;
    };

    void Renderer::EndScene()
    {
        FlushLines();
    };

    void Renderer::Init()
    {
        PROFILE_FUNCTION();

        RenderCommand::Init();
        ShaderLibrary::Init();

        ShaderLibrary::Get("Texture")->Bind();

        s_Data.CameraUniformBuffer = UniformBuffer::Create(nullptr, sizeof(CameraData));
        s_Data.CameraUniformBuffer->Bind(0);

        s_Data.LightUniformBuffer = UniformBuffer::Create(nullptr, sizeof(SceneLightData));
        s_Data.LightUniformBuffer->Bind(1);

        s_LineData.Init();

        switch (Renderer::Get())
        {
            case RendererAPI::API::None:
                break;

            case RendererAPI::API::OpenGL:
            {
                break;
            }
            case RendererAPI::API::Vulkan:
            {
                break;
            }
            default:
                break;
        }
    }

    void Renderer::Shutdown()
    {
        s_LineData.Shutdown();
        ShaderLibrary::Shutdown();
    }

    void Renderer::OnWindowResize(unsigned int width, unsigned int height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    };

    void Renderer::UpdateLightBuffer(SceneLightData& lightData)
    {
        s_Data.LightUniformBuffer->Update(0, sizeof(SceneLightData), &lightData);
    }

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
        s_LineData.LineShader->SetMat4("u_ViewProjection", s_SceneData.ViewProjectionMatrix);

        RenderCommand::DrawLines(s_LineData.LineVertexArray, s_LineData.VertexCount);

        s_LineData.ResetBatch();
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

    void Renderer::SubmitMesh(const Shared<Mesh> &mesh, const glm::mat4 &transform)
    {
        uintptr_t currentMaterialKey = reinterpret_cast<uintptr_t>(mesh.get());
        if (currentMaterialKey != s_SceneData.s_LastBoundMaterial)
        {
            //SetTexture(mesh->GetTextureLayout()); 
            
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
        if (count < 1) return;

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
