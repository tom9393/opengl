#include "../include/Renderer.h"

Renderer::SceneData Renderer::m_SceneData;
RenderStorage Renderer::m_Data;

void Renderer::BeginScene(std::shared_ptr<Camera> camera)
{
    m_SceneData.ViewProjectionMatrix = camera->GetViewProjectionMatrix();
    std::shared_ptr<Shader> shader = m_Data.shaderLibrary->Get("Texture");
    shader->Bind();
    shader->setMat4("u_ViewProjection", m_SceneData.ViewProjectionMatrix);
};

void Renderer::EndScene()
{
};

void Renderer::Init()
{
    PROFILE_FUNCTION();

    RenderCommand::Init();

    const std::string vert = "/home/tom/Coding/c++/opengl/shaders/default.vert";
    const std::string frag = "/home/tom/Coding/c++/opengl/shaders/default.frag";
    m_Data.shaderLibrary = new ShaderLibrary({"Texture"});
    m_Data.shaderLibrary->AddToShader("Texture", {vert, frag});
};

void Renderer::OnWindowResize(unsigned int width, unsigned int height)
{
    RenderCommand::SetViewport(0, 0, width, height);
};

void Renderer::SetTexture(const std::shared_ptr<Mesh> &mesh)
{
    PROFILE_FUNCTION();

    std::shared_ptr<Shader> shader = m_Data.shaderLibrary->Get("Texture");
    TextureLayout textureLayout = mesh->GetTextureLayout();

    unsigned int idx = 0;
    for (auto &textureType : textureLayout)
    {
        TextureType type = textureType.first;
        std::vector<std::shared_ptr<Texture2D>> textures = textureType.second;
        for (auto &texture : textures)
        {
            texture->Bind(idx);
            shader->setInt(textureLayout.GetStringByType(type), idx++);
            shader->setFloat("material.shininess", 0.75f);
        }
    }
};

void Renderer::Submit(const std::shared_ptr<Mesh> &mesh, glm::mat4 &transform)
{
    PROFILE_FUNCTION();

    m_Data.batch = new Batch(mesh->GetBufferLayout());

    if (m_Data.batch->GetLayout() == mesh->GetBufferLayout())
    {
        m_Data.batch->AddIndices(mesh->GetIndices());
        m_Data.batch->AddVertices(mesh->GetVertices());
        m_Data.batch->Prepare();
        std::shared_ptr<VertexArray> VAO = m_Data.batch->GetVAO();
        VAO->Bind();
        RenderCommand::DrawIndexedInstances(VAO, 100);
        delete m_Data.batch;
        m_Data.batch = nullptr;
        m_Data.statistics.DrawCalls++;
        m_Data.statistics.VerticeCount += mesh->m_Vertices.size();
        m_Data.statistics.IndicesCount += mesh->m_Indices.size();
    } else
    {
        std::cout << "Buffer layout mismatch" << std::endl;
    }
};

void Renderer::ToggleWireframe()
{
    RenderCommand::ToggleWireframe();
};
