#include "../include/Buffer.h"
#include "../include/Renderer.h"
#include "../platform/OpenGL/OpenGLBuffer.h"

std::shared_ptr<IndexBuffer> IndexBuffer::Create(unsigned int* indices, unsigned int size)
{
        switch (Renderer::Get()) {
                case RendererAPI::API::None: return nullptr;
                case RendererAPI::API::OpenGL: return std::make_shared<OpenGLIndexBuffer>(indices, size);
        }
    return nullptr;
};

std::shared_ptr<VertexBuffer> VertexBuffer::Create(unsigned int size)
{
        switch (Renderer::Get()) {
                case RendererAPI::API::None: return nullptr;
                case RendererAPI::API::OpenGL: return std::make_shared<OpenGLVertexBuffer>(size);
        }
    return nullptr;
};

std::shared_ptr<VertexBuffer> VertexBuffer::Create(float *vertices, unsigned int size)
{
        switch (Renderer::Get()) {
                case RendererAPI::API::None: return nullptr;
                case RendererAPI::API::OpenGL: return std::make_shared<OpenGLVertexBuffer>(vertices, size);
        }
    return nullptr;
};

std::shared_ptr<VertexArray> VertexArray::Create()
{
    switch (Renderer::Get()) {
        case RendererAPI::API::None: return nullptr;
        case RendererAPI::API::OpenGL: return std::make_shared<OpenGLVertexArray>();
        }
    return nullptr;
};

std::shared_ptr<SSBOBuffer> SSBOBuffer::Create(void* data, unsigned int size)
{
    switch (Renderer::Get()) {
        case RendererAPI::API::None: return nullptr;
        case RendererAPI::API::OpenGL: return std::make_shared<OpenGLSSBOBuffer>(data, size);
        }
    return nullptr;
};

std::shared_ptr<Framebuffer> Framebuffer::Create(FramebufferSpecification &specification)
{
    switch (Renderer::Get()) {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return std::make_shared<OpenGLFramebuffer>(specification);
    }
    return nullptr;
};
