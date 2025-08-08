#include "../include/Renderer.h"

Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;
RenderStorage* Renderer::m_Data = new RenderStorage;

void Renderer::BeginScene(std::shared_ptr<Camera> camera)
{
    m_SceneData->ViewProjectionMatrix = camera->GetViewProjectionMatrix();
};

void Renderer::EndScene()
{
    std::shared_ptr<Shader> shader = m_Data->shaderLibrary->Get("Texture");
    shader->Bind();
    shader->setMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
    if (!m_Data->set)
    {
        m_Data->set = true;
        for (auto &txt : m_Data->openglBindlessTextures)
        {
            txt->Bind();
        }
        m_Data->textureBuffer = SSBOBuffer::Create((void*)m_Data->textures.data(), sizeof(GLuint64) * m_Data->textures.size());
        m_Data->matrixBuffer->Bind(0);
        m_Data->textureBuffer->Bind(1);
        std::cout << "Here I am " << m_Data->textures.size() << std::endl;
    }

    if (!m_Data->batch->IsReady()) m_Data->batch->Prepare();
    std::shared_ptr<VertexArray> VAO = m_Data->batch->GetVAO();
    VAO->Bind();
    RenderCommand::DrawIndexedInstances(VAO, 100);
};

void Renderer::Init()
{
    PROFILE_FUNCTION();

    RenderCommand::Init();

    const std::string vert = "/home/tom/Coding/c++/opengl/shaders/default.vert";
    const std::string frag = "/home/tom/Coding/c++/opengl/shaders/default.frag";
    m_Data->shaderLibrary = new ShaderLibrary({"Texture"});
    m_Data->shaderLibrary->AddToShader("Texture", {vert, frag});

    std::vector<glm::mat4> instancedModelMatrices;
    unsigned int c = 10;
    for (unsigned int i = 0; i < c; i++)
    {
        for (unsigned int j = 0; j < c; j++)
        {
            glm::vec3 v = {i * 4.0f, 0.0f, j * 10.0f};
            glm::mat4 mat = glm::translate(glm::mat4(1.0f), v);
            instancedModelMatrices.emplace_back(mat);
        }
    }

    m_Data->matrixBuffer = SSBOBuffer::Create((void*)instancedModelMatrices.data(), sizeof(glm::mat4) * instancedModelMatrices.size());
};

void Renderer::OnWindowResize(unsigned int width, unsigned int height)
{
    RenderCommand::SetViewport(0, 0, width, height);
};

void Renderer::SetTexture(const std::shared_ptr<Texture> &texture, const std::unordered_map<std::string, std::shared_ptr<Texture2D>> &textures)
{
    if (m_Data->set) return;
    PROFILE_FUNCTION();

    for (auto &tx : textures)
    {
        std::shared_ptr<BindlessTexture> txt = BindlessTexture::Create();
        auto opengltxt = std::dynamic_pointer_cast<OpenGLBindlessTexture>(txt);
        auto opengltexture = std::dynamic_pointer_cast<OpenGLTexture2D>(tx.second);
        opengltxt->MakeHandle(opengltexture);
        GLuint64 textureHandle = opengltxt->GetHandle();
        std::cout << "Pushing handle " << tx.first << std::endl;
        m_Data->textures.push_back(textureHandle);
        m_Data->openglBindlessTextures.push_back(opengltxt);
    }

    // std::shared_ptr<Shader> shader = m_Data->shaderLibrary->Get("Texture");
    // texture->Bind(0);
    // std::string textType = "";
    // TextureType txtType = std::dynamic_pointer_cast<OpenGLTexture2D>(texture)->GetType();
    // if (txtType == TextureType::Diffuse) textType = "material.diffuse";
    // else if (txtType == TextureType::Specular) textType = "material.specular";
    // shader->setInt(textType, 0);
    // shader->setFloat("material.shininess", 0.75f);
};

void Renderer::Submit(const std::shared_ptr<Mesh> &mesh, const glm::mat4 &transform)
{
    PROFILE_FUNCTION();

    if (!m_Data->batch) m_Data->batch = new Batch(mesh->GetLayout());
    if (m_Data->batch->IsReady()) return;

    if (m_Data->batch->GetLayout() == mesh->GetLayout())
    {
        m_Data->batch->AddIndices(mesh->GetIndices());
        m_Data->batch->AddVertices(mesh->GetVertices());
    } else
    {
        std::cout << "Buffer layout mismatch" << std::endl;
    }
};

void Renderer::ToggleWireframe()
{
    RenderCommand::ToggleWireframe();
};
