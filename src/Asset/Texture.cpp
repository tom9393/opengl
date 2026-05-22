#include "OpenGL/OpenGLTexture.h"
#include "TextureImporter.h"
#include "Render/Renderer.h"
#include "Texture.h"

namespace Engine
{

    std::shared_ptr<Texture2D> Texture2D::Create(const TextureLoadData &data)
    {
        switch (Renderer::Get()) {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return std::make_shared<OpenGLTexture2D>(data);
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