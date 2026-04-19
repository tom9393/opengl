#include "Renderer.h"
#include "../platform/OpenGL/OpenGLShader.h"
#include "Buffer.h"
#include "../Utils/MathTools.h"

namespace Engine
{

    Renderer::SceneData Renderer::s_SceneData;
    RenderStorage Renderer::s_Data;
    LineData Renderer::s_LineData;

    void Renderer::BeginScene()
    {
        // s_SceneData.ViewProjectionMatrix = data.ViewProjection;
	    // s_SceneData.cameraFrustum = CreateFrustumFromMatrix(data.ViewProjection);
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

    void Renderer::SetMaterial(const Shared<Material> &material)
    {
        PROFILE_FUNCTION();

        if (!material) return;
        material->Bind();
    }

    void Renderer::FlushLines()
    {
        // if (s_LineData.VertexCount == 0) return; // Fix this later

        // s_LineData.LineVertexBuffer->SetData(s_LineData.VertexBufferBase, s_LineData.GetDataSize());

        // s_LineData.LineShader->Bind();
        // s_LineData.LineShader->SetMat4("u_ViewProjection", s_SceneData.ViewProjectionMatrix);

        // RenderCommand::DrawLines(s_LineData.LineVertexArray, s_LineData.VertexCount);

        // s_LineData.ResetBatch();
        // s_Data.statistics.DrawCalls++;
    }

    void Renderer::DrawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color)
    {
        if (s_LineData.IsFull()) {
            FlushLines();
        }

        s_LineData.AddVertex(p0, color);
        s_LineData.AddVertex(p1, color);
    }

    static uint32_t s_QuadVAO = 0;
    static uint32_t s_QuadVBO = 0;

    void Renderer::DrawFullscreenQuad()
    {
        if (s_QuadVAO == 0)
        {
            // A fullscreen quad defined in Normalized Device Coordinates (NDC)
            // spanning from -1 to 1 on both X and Y axes.
            float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
                 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
                 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
            };
            
            glGenVertexArrays(1, &s_QuadVAO);
            glGenBuffers(1, &s_QuadVBO);
            
            glBindVertexArray(s_QuadVAO);
            
            glBindBuffer(GL_ARRAY_BUFFER, s_QuadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            
            // Layout location 0: a_Position (vec3)
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            
            // Layout location 1: a_TexCoords (vec2)
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        }
        
        // Bind the VAO and draw the quad as a triangle strip
        glBindVertexArray(s_QuadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
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
