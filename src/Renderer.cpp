#include "Render/Renderer.h"
#include "../platform/OpenGL/OpenGLShader.h"
#include "Buffer.h"
#include "MathTools.h"

namespace Engine
{

    Renderer::SceneData Renderer::s_SceneData;
    RenderStorage Renderer::s_Data;
    LineRendererData Renderer::s_LineData;

    void Renderer::BeginScene(CameraData& cameraData)
    {
        Shared<Shader> shader = s_Data.shaderLibrary->Get("Texture");
        shader->Bind();

        s_Data.uniformBuffer->Update(0, sizeof(CameraData), &cameraData);
        s_SceneData.ViewProjectionMatrix = cameraData.ViewProjection;
	s_SceneData.cameraFrustum = CreateFrustumFromMatrix(cameraData.ViewProjection);
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

        // Generic setup: Create the shader library and uniform buffer.
        // Note: Buffer::Create methods usually handle API-specific implementation internally.
        
        // Main program
        s_Data.shaderLibrary = new ShaderLibrary({"Texture"});
        
        const std::string vert = "/home/tom/Coding/c++/opengl/shaders/main.vert";
        const std::string frag = "/home/tom/Coding/c++/opengl/shaders/main.frag";
        s_Data.shaderLibrary->AddToShader("Texture", {vert, frag});

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

        Shared<Shader> shader = s_Data.shaderLibrary->Get("Texture");
        shader->Bind();

        // Assuming you have a way to set arrays in your shader class:
        for (unsigned int i = 0; i < 4; ++i) {
            shader->setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
            shader->setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
        }

        s_Data.uniformBuffer = UniformBuffer::Create(nullptr, sizeof(CameraData));
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
        s_LineData.LineShader = Shader::Create("LineShader");
        s_LineData.LineShader->AddShaders({vert_line, frag_line});

        switch (Renderer::Get())
        {
            case RendererAPI::API::None:
                break;

            case RendererAPI::API::OpenGL:
            {
                Shared<Shader> shader = s_Data.shaderLibrary->Get("Texture");
                auto openglShader = std::dynamic_pointer_cast<OpenGLShader>(shader);
                
                if (openglShader)
                {
                    openglShader->Bind();
                    unsigned int viewprojectionIndex = glGetUniformBlockIndex(openglShader->GetProgram(), "Camera");
                    glUniformBlockBinding(openglShader->GetProgram(), viewprojectionIndex, 0);
                }
                break;
            }

            /* case RendererAPI::API::Vulkan:
            {
                // Add Vulkan-specific initialization here
                // e.g., descriptor set layout creation, pipeline state setup, etc.
                break;
            }
            */

            default:
                break;
        }
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
                shader->setInt(layout.GetStringByType(type), idx++);
            }
        }
    };

    void Renderer::Submit(const Shared<Mesh> &mesh, const glm::mat4 &transform)
    {
        PROFILE_FUNCTION();

        s_Data.shaderLibrary->Get("Texture")->setMat4("u_Transform", transform);

        Shared<VertexArray> VAO = mesh->GetVAO();
        VAO->Bind();
        RenderCommand::DrawIndexed(VAO);
        s_Data.statistics.DrawCalls++;
        s_Data.statistics.VerticeCount += mesh->m_Vertices.size();
        s_Data.statistics.IndicesCount += mesh->m_Indices.size();
    };

    void Renderer::FlushLines()
    {
        if (s_LineData.LineVertexCount == 0) return;

        uint32_t dataSize = (uint32_t)((uint8_t*)s_LineData.LineVertexBufferPtr - (uint8_t*)s_LineData.LineVertexBufferBase);
        s_LineData.LineVertexBuffer->SetData(s_LineData.LineVertexBufferBase, dataSize);

        s_LineData.LineShader->Bind();
        s_LineData.LineShader->setMat4("u_ViewProjection", s_SceneData.ViewProjectionMatrix);

        glLineWidth(1.0f); 
        RenderCommand::DrawLines(s_LineData.LineVertexArray, s_LineData.LineVertexCount);

        s_LineData.LineVertexCount = 0;
        s_LineData.LineVertexBufferPtr = s_LineData.LineVertexBufferBase;
        s_Data.statistics.DrawCalls++;
    }

    void Renderer::DrawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color)
    {
        // If buffer is full, flush and start over
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
            // This is where your heavy OpenGL calls happen 
            // (e.g., binding Albedo, Normal, Metallic to Texture Units)
            SetTexture(mesh->GetTextureLayout()); 
            
            s_SceneData.s_LastBoundMaterial = currentMaterialKey;
            s_Data.statistics.MaterialSwitches++;
        }

        Submit(mesh, transform);
    }

    void Renderer::ToggleWireframe()
    {
        RenderCommand::ToggleWireframe();
    };

};
