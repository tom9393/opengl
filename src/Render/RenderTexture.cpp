#include "RenderTexture.h"
#include "Renderer.h"
#include "OpenGL/OpenGLRenderTexture.h"

namespace Engine
{

    Shared<RenderTexture> RenderTexture::Create(const RenderTextureSpecification& spec)
    {
        switch (Renderer::Get())
        {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return MakeShared<OpenGLRenderTexture>(spec);
        }
        return nullptr;
    }

}
