#include "Texture.h"
#include "Render/Renderer.h"
#include "../platform/OpenGL/OpenGLTexture.h"

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

    std::shared_ptr<Texture2D> Texture2D::Create(void* data, size_t size)
    {
        switch (Renderer::Get()) {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return std::make_shared<OpenGLTexture2D>(data, size);
            }
        return nullptr;

    };

    std::shared_ptr<Texture2D> Texture2D::Create(void* data, uint32_t width, uint32_t height)
    {
        switch (Renderer::Get()) {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return std::make_shared<OpenGLTexture2D>(data, width, height);
            }
        return nullptr;

    };

    std::shared_ptr<Texture2D> Texture2D::Create(uint32_t rendererID, uint32_t width, uint32_t height)
    {
        switch (Renderer::Get()) {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return std::make_shared<OpenGLTexture2D>(rendererID, width, height);
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