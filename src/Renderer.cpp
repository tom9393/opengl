#include "../include/Renderer.h"
#include "../platform/OpenGL/OpenGLShader.h"
#include <memory>

Renderer::SceneData Renderer::m_SceneData;
RenderStorage Renderer::m_Data;

void Renderer::BeginScene(const glm::mat4& viewProjection)
{
    std::shared_ptr<Shader> shader = m_Data.shaderLibrary->GetLatest();
    auto openglshader = std::dynamic_pointer_cast<OpenGLShader>(shader);
    openglshader->Bind();

    m_Data.uniformBuffer->Update(0, sizeof(glm::mat4), (void*)glm::value_ptr(viewProjection));

    int size = 0;
    glGetNamedBufferParameteriv(openglshader->GetProgram(), GL_BUFFER_SIZE, &size);
    std::cout << "UBO size after is: " << size << std::endl;
};

void Renderer::EndScene()
{
};

void Renderer::Init()
{
    PROFILE_FUNCTION();

    RenderCommand::Init();

    const std::string vert = "/home/tom/Coding/c++/opengl/shaders/main.vert";
    const std::string frag = "/home/tom/Coding/c++/opengl/shaders/main.frag";
    m_Data.shaderLibrary = new ShaderLibrary({"Texture"});
    m_Data.shaderLibrary->AddToShader("Texture", {vert, frag});
    m_Data.shaderLibrary->SetLatest("Texture");

    m_Data.uniformBuffer = UniformBuffer::Create(nullptr, sizeof(glm::mat4));
    m_Data.uniformBuffer->Bind(0);

    std::shared_ptr<Shader> shader = m_Data.shaderLibrary->GetLatest();
    auto openglshader = std::dynamic_pointer_cast<OpenGLShader>(shader);
    openglshader->Bind();

    unsigned int viewprojectionIndex = glGetUniformBlockIndex(openglshader->GetProgram(), "Camera");
    glUniformBlockBinding(openglshader->GetProgram(), viewprojectionIndex, 0);
};

void Renderer::OnWindowResize(unsigned int width, unsigned int height)
{
    RenderCommand::SetViewport(0, 0, width, height);
};

void Renderer::SetTexture(TextureLayout &layout)
{
    PROFILE_FUNCTION();

    std::shared_ptr<Shader> shader = m_Data.shaderLibrary->GetLatest();
    unsigned int idx = 0;
    for (auto &textureType : layout)
    {
        TextureType type = textureType.first;
        std::vector<std::shared_ptr<Texture2D>> textures = textureType.second;
        for (auto &texture : textures)
        {
            texture->Bind(idx);
            shader->setInt(layout.GetStringByType(type), idx++);
            shader->setFloat("material.shininess", 0.75f);
        }
    }
};

void Renderer::Submit(const std::shared_ptr<Mesh> &mesh, glm::mat4 &transform)
{
    PROFILE_FUNCTION();

    m_Data.shaderLibrary->GetLatest()->setMat4("u_Transform", transform);

    std::shared_ptr<VertexArray> VAO = mesh->GetVAO();
    VAO->Bind();
    RenderCommand::DrawIndexed(VAO);
    m_Data.statistics.DrawCalls++;
    m_Data.statistics.VerticeCount += mesh->m_Vertices.size();
    m_Data.statistics.IndicesCount += mesh->m_Indices.size();
};

void Renderer::Submit(const std::shared_ptr<Mesh> &mesh, glm::mat4 &transform, int id)
{
    PROFILE_FUNCTION();

    m_Data.shaderLibrary->GetLatest()->setMat4("u_Transform", transform);
    m_Data.shaderLibrary->GetLatest()->setInt("u_EntityID", id);

    std::shared_ptr<VertexArray> VAO = mesh->GetVAO();
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
