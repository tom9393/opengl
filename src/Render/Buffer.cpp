#include "Buffer.h"
#include "Render/Renderer.h"
#include "OpenGL/OpenGLBuffer.h"

namespace Engine
{

    Shared<IndexBuffer> IndexBuffer::Create(unsigned int* indices, unsigned int size)
    {
            switch (Renderer::Get()) {
                    case RendererAPI::API::None: return nullptr;
                    case RendererAPI::API::OpenGL: return MakeShared<OpenGLIndexBuffer>(indices, size);
            }
        return nullptr;
    };

    Shared<VertexBuffer> VertexBuffer::Create(unsigned int size)
    {
            switch (Renderer::Get()) {
                    case RendererAPI::API::None: return nullptr;
                    case RendererAPI::API::OpenGL: return MakeShared<OpenGLVertexBuffer>(size);
            }
        return nullptr;
    };

    Shared<VertexBuffer> VertexBuffer::Create(float *vertices, unsigned int size)
    {
            switch (Renderer::Get()) {
                    case RendererAPI::API::None: return nullptr;
                    case RendererAPI::API::OpenGL: return MakeShared<OpenGLVertexBuffer>(vertices, size);
            }
        return nullptr;
    };

    Shared<VertexArray> VertexArray::Create()
    {
        switch (Renderer::Get()) {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return MakeShared<OpenGLVertexArray>();
            }
        return nullptr;
    };

    Shared<UniformBuffer> UniformBuffer::Create(void* data, unsigned int size)
    {
        switch (Renderer::Get()) {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return MakeShared<OpenGLUniformBuffer>(data, size);
            }
        return nullptr;
    };

    Shared<ShaderStorageBuffer> ShaderStorageBuffer::Create(unsigned int size)
    {
        switch (Renderer::Get()) {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return MakeShared<OpenGLShaderStorageBuffer>(size);
            }
        return nullptr;
    };

    Shared<ShaderStorageBuffer> ShaderStorageBuffer::Create(void* data, unsigned int size)
    {
        switch (Renderer::Get()) {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return MakeShared<OpenGLShaderStorageBuffer>(data, size);
            }
        return nullptr;
    };

    Shared<RenderTarget> RenderTarget::Create(const RenderTargetSpecification& specification)
    {
        switch (Renderer::Get()) {
                case RendererAPI::API::None: return nullptr;
                case RendererAPI::API::OpenGL: return MakeShared<OpenGLRenderTarget>(specification);
        }
        return nullptr;
    };

    Shared<Framebuffer> Framebuffer::Create(const FramebufferSpecification& specification)
    {
        switch (Renderer::Get()) {
                case RendererAPI::API::None: return nullptr;
                case RendererAPI::API::OpenGL: return MakeShared<OpenGLFramebuffer>(specification);
        }
        return nullptr;
    };

};
