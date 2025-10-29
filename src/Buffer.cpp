#include "../include/Buffer.h"
#include "../include/Renderer.h"
#include "../platform/OpenGL/OpenGLBuffer.h"

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

Shared<SSBOBuffer> SSBOBuffer::Create(void* data, unsigned int size)
{
    switch (Renderer::Get()) {
        case RendererAPI::API::None: return nullptr;
        case RendererAPI::API::OpenGL: return MakeShared<OpenGLSSBOBuffer>(data, size);
        }
    return nullptr;
};

Shared<Framebuffer> Framebuffer::Create(FramebufferSpecification &specification)
{
    switch (Renderer::Get()) {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return MakeShared<OpenGLFramebuffer>(specification);
    }
    return nullptr;
};
