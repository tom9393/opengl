#include "../include/Shader.h"
#include "../include/Renderer.h"
#include "../platform/OpenGL/OpenGLShader.h"

std::shared_ptr<Shader> Shader::Create(const std::string &name)
{
    switch (Renderer::Get()) {
        case RendererAPI::API::None: return nullptr;
        case RendererAPI::API::OpenGL: return std::make_shared<OpenGLShader>(name);
        }
    return nullptr;
};

void ShaderLibrary::Add(const std::shared_ptr<Shader> &shader)
{
    auto &name = shader->GetName();
    m_Shaders[name] = shader;
};

std::shared_ptr<Shader> ShaderLibrary::Get(const std::string &name)
{
    return m_Shaders[name];
};
