#include "Render/Renderer.h"
#include "../platform/OpenGL/OpenGLShader.h"
#include "Buffer.h"

namespace Engine
{

    Renderer::SceneData Renderer::s_SceneData;
    RenderStorage Renderer::s_Data;
    LineRendererData Renderer::s_LineData;

    void Renderer::BeginScene(const glm::mat4& viewProjection)
    {
        Shared<Shader> shader = s_Data.shaderLibrary->Get("Texture");
        shader->Bind();

        s_Data.uniformBuffer->Update(0, sizeof(glm::mat4), (void*)glm::value_ptr(viewProjection));
        s_SceneData.ViewProjectionMatrix = viewProjection;
    };

    void Renderer::EndScene()
    {
        for (auto &renderCall : s_Data.renderQueue)
        {
            SetTexture(renderCall.mesh->GetTextureLayout());
            Submit(renderCall.mesh, renderCall.transform);
        }
        s_Data.renderQueue.clear();

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

        s_Data.uniformBuffer = UniformBuffer::Create(nullptr, sizeof(glm::mat4));
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
                shader->setFloat("material.shininess", 0.75f);
            }
        }
    };

    void Renderer::Submit(const Shared<Mesh> &mesh, glm::mat4 &transform)
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

        // Set line width (optional, depends on driver support)
        glLineWidth(5.0f); 
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

    void Renderer::SubmitModel(const Shared<Model> &model, const glm::mat4 &transform)
    {
        PROFILE_FUNCTION();
        for (auto& mesh : model->GetMeshes())
        {
            // Batching or Material sorting can happen here
            s_Data.renderQueue.emplace_back(mesh, transform);
        }
    };

    void Renderer::ToggleWireframe()
    {
        RenderCommand::ToggleWireframe();
    };

};
