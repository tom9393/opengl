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

    std::unordered_map<std::string, Shared<Shader>> ShaderLibrary::s_Shaders;

    void ShaderLibrary::Init()
    {
        Shared<Shader> textureShader = Shader::Create("Texture");
        const std::string vert = "/home/tom/Coding/c++/opengl/shaders/main.vert";
        const std::string frag = "/home/tom/Coding/c++/opengl/shaders/main.frag";
        textureShader->AddShaders({vert, frag});
        Add(textureShader);

        Shared<Shader> primitiveShader = Shader::Create("Primitive");
        const std::string pvert = "/home/tom/Coding/c++/opengl/shaders/primitive.vert";
        const std::string pfrag = "/home/tom/Coding/c++/opengl/shaders/primitive.frag";
        primitiveShader->AddShaders({pvert, pfrag});
        Add(primitiveShader);

        Shared<Shader> shadowShader = Shader::Create("Shadow");
        const std::string svert = "/home/tom/Coding/c++/opengl/shaders/shadow.vert";
        const std::string sfrag = "/home/tom/Coding/c++/opengl/shaders/shadow.frag";
        primitiveShader->AddShaders({svert, sfrag});
        Add(shadowShader);
    }

    void ShaderLibrary::Shutdown()
    {

    }

    void ShaderLibrary::Add(const Shared<Shader> &shader)
    {
        auto &name = shader->GetName();
        s_Shaders[name] = shader;
    };

    Shared<Shader> ShaderLibrary::Get(const std::string &name)
    {
        return s_Shaders[name];
    };

    void ShaderLibrary::AddToShader(const std::string &shader, const std::initializer_list<std::string> &paths)
    {
        s_Shaders[shader]->AddShaders(paths);
    };

    ShaderLibrary::~ShaderLibrary()
    {
    }

};
