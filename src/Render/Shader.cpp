#include "Render/Shader.h"
#include "Render/Renderer.h"
#include "../platform/OpenGL/OpenGLShader.h"

namespace Engine
{

    Shared<Shader> Shader::Create(const std::string &name)
    {
        switch (Renderer::Get()) {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return std::make_shared<OpenGLShader>(name);
            }
        return nullptr;
    };

    void ShaderLibrary::Add(const Shared<Shader> &shader)
    {
        auto &name = shader->GetName();
        m_Shaders[name] = shader;
    };

    Shared<Shader> ShaderLibrary::Get(const std::string &name)
    {
        return m_Shaders[name];
    };

    ShaderLibrary::ShaderLibrary(const std::initializer_list<std::string> &shaders)
    {
        for (auto &shader : shaders)
        {
            m_Shaders[shader] = Shader::Create(shader);
        }
    };

    ShaderLibrary::ShaderLibrary()
    {
        Shared<Shader> textureShader = Shader::Create("Texture");
        const std::string vert = "/home/tom/Coding/c++/opengl/shaders/main.vert";
        const std::string frag = "/home/tom/Coding/c++/opengl/shaders/main.frag";
        textureShader->AddShaders({vert, frag});
        Add(textureShader);
    }

    void ShaderLibrary::AddToShader(const std::string &shader, const std::initializer_list<std::string> &paths)
    {
        m_Shaders[shader]->AddShaders(paths);
    };

    ShaderLibrary::~ShaderLibrary()
    {
    }

};
