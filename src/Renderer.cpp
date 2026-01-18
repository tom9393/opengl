#include "Render/Renderer.h"
#include "../platform/OpenGL/OpenGLShader.h"

namespace Engine
{

    Renderer::SceneData Renderer::m_SceneData;
    RenderStorage Renderer::m_Data;

    void Renderer::BeginScene(const glm::mat4& viewProjection)
    {
        Shared<Shader> shader = m_Data.shaderLibrary->Get("Texture");
        shader->Bind();

        m_Data.uniformBuffer->Update(0, sizeof(glm::mat4), (void*)glm::value_ptr(viewProjection));
    };

    void Renderer::EndScene()
    {
    };

    void Renderer::Init()
    {
        PROFILE_FUNCTION();

        RenderCommand::Init();

        // Generic setup: Create the shader library and uniform buffer.
        // Note: Buffer::Create methods usually handle API-specific implementation internally.
        m_Data.shaderLibrary = new ShaderLibrary({"Texture"});
        
        const std::string vert = "/home/tom/Coding/c++/opengl/shaders/main.vert";
        const std::string frag = "/home/tom/Coding/c++/opengl/shaders/main.frag";
        m_Data.shaderLibrary->AddToShader("Texture", {vert, frag});

        m_Data.uniformBuffer = UniformBuffer::Create(nullptr, sizeof(glm::mat4));
        m_Data.uniformBuffer->Bind(0);

        switch (Renderer::Get())
        {
            case RendererAPI::API::None:
                break;

            case RendererAPI::API::OpenGL:
            {
                Shared<Shader> shader = m_Data.shaderLibrary->Get("Texture");
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

        Shared<Shader> shader = m_Data.shaderLibrary->Get("Texture");
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

        m_Data.shaderLibrary->Get("Texture")->setMat4("u_Transform", transform);

        Shared<VertexArray> VAO = mesh->GetVAO();
        VAO->Bind();
        RenderCommand::DrawIndexed(VAO);
        m_Data.statistics.DrawCalls++;
        m_Data.statistics.VerticeCount += mesh->m_Vertices.size();
        m_Data.statistics.IndicesCount += mesh->m_Indices.size();
    };

    void Renderer::ToggleWireframe()
    {
        RenderCommand::ToggleWireframe();
    };

};
