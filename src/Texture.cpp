#include "../include/Texture.h"
#include "../include/Render/Renderer.h"

namespace Engine
{

    std::shared_ptr<Texture2D> Texture2D::Create(const std::string &path, TextureType type)
    {
        switch (Renderer::Get()) {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return std::make_shared<OpenGLTexture2D>(path, type);
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
