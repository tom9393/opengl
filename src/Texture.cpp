#include "../include/Texture.h"
#include "../include/Renderer.h"

#include "../platform/OpenGL/OpenGLTexture.h"

std::shared_ptr<Texture2D> Texture2D::Create(const std::string &path, TextureType type)
{
    switch (Renderer::Get()) {
        case RendererAPI::API::None: return nullptr;
        case RendererAPI::API::OpenGL: return std::make_shared<OpenGLTexture2D>(path, type);
        }
    return nullptr;

};
