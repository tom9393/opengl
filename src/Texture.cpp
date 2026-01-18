#include "Texture.h"
#include "Render/Renderer.h"

namespace Engine
{

    std::shared_ptr<Texture2D> Texture2D::Create(const std::filesystem::path &path)
    {
        switch (Renderer::Get()) {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return std::make_shared<OpenGLTexture2D>(path);
            }
        return nullptr;

    };

    std::shared_ptr<BindlessTexture> BindlessTexture::Create()
    {
        switch (Renderer::Get()) {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return std::make_shared<OpenGLBindlessTexture>();
            }
        return nullptr;

    };

};
