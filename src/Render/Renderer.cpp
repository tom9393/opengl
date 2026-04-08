#include "Renderer.h"
#include "../platform/OpenGL/OpenGLShader.h"
#include "Buffer.h"
#include "../Utils/MathTools.h"

namespace Engine
{

    Renderer::SceneData Renderer::s_SceneData;
    RenderStorage Renderer::s_Data;
    LineRendererData Renderer::s_LineData;

    void Renderer::BeginScene(RenderContext &context)
    {
        Shared<Shader> shader = s_Data.shaderLibrary->Get("Texture");
        shader->Bind();

        CameraData data;
        data.ViewProjection = context.ViewProjection;
        data.CameraPosition = context.CameraPosition;

        s_Data.uniformBuffer->Update(0, sizeof(data), &data);
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

        s_Data.shaderLibrary = new ShaderLibrary();

        Shared<Shader> textureShader = Shader::Create("Texture");
        const std::string vert = "/home/tom/Coding/c++/opengl/shaders/main.vert";
        const std::string frag = "/home/tom/Coding/c++/opengl/shaders/main.frag";
        textureShader->AddShaders({vert, frag});
        s_Data.shaderLibrary->Add(textureShader);

        glm::vec3 lightPositions[] = {
            glm::vec3(-10.0f,  10.0f, 10.0f),
            glm::vec3( 10.0f,  10.0f, 10.0f),
            glm::vec3(-10.0f, -10.0f, 10.0f),
            glm::vec3( 10.0f, -10.0f, 10.0f),
        };
        float intensity = 1000.0f;
        glm::vec3 lightColors[] = {
            glm::vec3(intensity, intensity, intensity),
            glm::vec3(intensity, intensity, intensity),
            glm::vec3(intensity, intensity, intensity),
            glm::vec3(intensity, intensity, intensity)
        };

        textureShader->Bind();

        // Assuming you have a way to set arrays in your shader class:
        for (unsigned int i = 0; i < 4; ++i) {
            textureShader->SetVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
            textureShader->SetVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
        }

        s_Data.uniformBuffer = UniformBuffer::Create(nullptr, sizeof(RenderContext));
        s_Data.uniformBuffer->Bind(0);

        // Mainly for drawing debugging lines
        s_LineData.LineVertexArray = VertexArray::Create();
        s_LineData.LineVertexBuffer = VertexBuffer::Create(s_LineData.MaxLines * sizeof(LineVertex));

        s_LineData.LineVertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color"    }
        });
        s_LineData.LineVertexArray->AddVertexBuffer(s_LineData.LineVertexBuffer);
	    s_LineData.LineVertexBufferBase = new LineVertex[s_LineData.MaxVertices];
        s_LineData.LineVertexCount = 0;
        s_LineData.LineVertexBufferPtr = s_LineData.LineVertexBufferBase;

        const std::string vert_line = "/home/tom/Coding/c++/opengl/shaders/line.vert";
        const std::string frag_line = "/home/tom/Coding/c++/opengl/shaders/line.frag";
        Shared<Shader> lineShader = Shader::Create("LineShader");
        lineShader->AddShaders({vert_line, frag_line});
        s_Data.shaderLibrary->Add(lineShader);

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
        delete s_Data.shaderLibrary;
    }

    void Renderer::OnWindowResize(unsigned int width, unsigned int height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    };

    void Renderer::SetTexture(TextureLayout &layout)
    {
        PROFILE_FUNCTION();

        Shared<Shader> shader = s_Data.shaderLibrary->Get("Texture");
        unsigned int idx = 0;
        for (auto &textureType : layout)
        {
            TextureType type = textureType.first;
            std::vector<Shared<Texture2D>> textures = textureType.second;
            for (auto &texture : textures)
            {
                texture->Bind(idx);
                shader->SetInt(layout.GetStringByType(type), idx++);
            }
        }
    };

    void Renderer::SetMaterial(const Shared<Material> &material)
    {
        PROFILE_FUNCTION();

        if (!material) return;

        // Fetch your PBR/Texture shader
        Shared<Shader> shader = s_Data.shaderLibrary->Get("Texture"); 

        unsigned int idx = 0;

        // --- 1. BIND TEXTURE MAPS ---
        
        if (material->AlbedoMap)
        {
            material->AlbedoMap->Bind(idx);
            shader->SetInt("pbr.albedoMap", idx);
            shader->SetBool("pbr.useAlbedoMap", true); // Highly recommended for your shader
            idx++;
        }
        else { shader->SetBool("pbr.useAlbedoMap", false); }

        if (material->NormalMap)
        {
            material->NormalMap->Bind(idx);
            shader->SetInt("pbr.normalMap", idx);
            shader->SetBool("pbr.useNormalMap", true);
            idx++;
        }
        else { shader->SetBool("pbr.useNormalMap", false); }

        if (material->MetallicMap)
        {
            material->MetallicMap->Bind(idx);
            shader->SetInt("pbr.metallicMap", idx);
            shader->SetBool("pbr.useMetallicMap", true);
            idx++;
        }
        else { shader->SetBool("pbr.useMetallicMap", false); }

        if (material->RoughnessMap)
        {
            material->RoughnessMap->Bind(idx);
            shader->SetInt("pbr.roughnessMap", idx);
            shader->SetBool("pbr.useRoughnessMap", true);
            idx++;
        }
        else { shader->SetBool("pbr.useRoughnessMap", false); }

        if (material->AOMap)
        {
            material->AOMap->Bind(idx);
            shader->SetInt("pbr.aoMap", idx);
            shader->SetBool("pbr.useAOMap", true);
            idx++;
        }
        else { shader->SetBool("pbr.useAOMap", false); }


        // --- 2. SET SCALAR / FALLBACK VALUES ---
        // Note: Make sure the function names (SetFloat4, SetFloat) match your Shader API
        
        shader->SetVec4("pbr.albedoTint", material->AlbedoTint);
        shader->SetFloat("pbr.metallicMultiplier", material->MetallicMultiplier);
        shader->SetFloat("pbr.roughnessMultiplier", material->RoughnessMultiplier);
        shader->SetFloat("pbr.emission", material->Emission);
    }

    void Renderer::FlushLines()
    {
        if (s_LineData.LineVertexCount == 0) return;

        uint32_t dataSize = (uint32_t)((uint8_t*)s_LineData.LineVertexBufferPtr - (uint8_t*)s_LineData.LineVertexBufferBase);
        s_LineData.LineVertexBuffer->SetData(s_LineData.LineVertexBufferBase, dataSize);

        Shared<Shader> shader = s_Data.shaderLibrary->Get("LineShader");

        shader->Bind();
        shader->SetMat4("u_ViewProjection", s_SceneData.ViewProjectionMatrix);

        glLineWidth(1.0f); 
        RenderCommand::DrawLines(s_LineData.LineVertexArray, s_LineData.LineVertexCount);

        s_LineData.LineVertexCount = 0;
        s_LineData.LineVertexBufferPtr = s_LineData.LineVertexBufferBase;
        s_Data.statistics.DrawCalls++;
    }

    void Renderer::DrawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color)
    {
        if (s_LineData.LineVertexCount + 2 >= s_LineData.MaxVertices)
            FlushLines();

        s_LineData.LineVertexBufferPtr->Position = p0;
        s_LineData.LineVertexBufferPtr->Color = color;
        s_LineData.LineVertexBufferPtr++;

        s_LineData.LineVertexBufferPtr->Position = p1;
        s_LineData.LineVertexBufferPtr->Color = color;
        s_LineData.LineVertexBufferPtr++;

        s_LineData.LineVertexCount += 2;
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

        s_Data.shaderLibrary->Get("Texture")->SetMat4("u_Transform", transform);

        Shared<VertexArray> VAO = mesh->GetVAO();
        VAO->Bind();
        RenderCommand::DrawIndexed(VAO);
        s_Data.statistics.DrawCalls++;
        s_Data.statistics.VerticeCount += mesh->GetVertices().size();
        s_Data.statistics.IndicesCount += mesh->GetIndices().size();
    }

    void Renderer::SubmitInstancedMesh(const Shared<Mesh> &mesh, const Shared<Material> &material, const std::vector<glm::mat4> &transforms)
    {
        if (transforms.empty()) return;

        uintptr_t currentMaterialKey = reinterpret_cast<uintptr_t>(material.get());
        if (currentMaterialKey != s_SceneData.s_LastBoundMaterial) {
            SetMaterial(material);
            
            s_SceneData.s_LastBoundMaterial = currentMaterialKey;
            s_Data.statistics.MaterialSwitches++;
        }

        Shared<VertexArray> VAO = mesh->GetVAO();
        VAO->Bind();
        RenderCommand::DrawIndexedInstances(VAO, transforms);
        s_Data.statistics.DrawCalls++;
        s_Data.statistics.VerticeCount += mesh->GetVertices().size();
        s_Data.statistics.IndicesCount += mesh->GetIndices().size();
    }

    void Renderer::ToggleWireframe()
    {
        RenderCommand::ToggleWireframe();
    };

};
