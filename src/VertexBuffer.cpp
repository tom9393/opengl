#include "../include/VertexBuffer.h"
#include "../include/Renderer.h"
#include "../platform/OpenGL/OpenGLVertexBuffer.h"

std::shared_ptr<VertexBuffer> VertexBuffer::Create(float *vertices, uint32_t size)
{
        switch (Renderer::Get()) {
                case RendererAPI::API::None: return nullptr;
                case RendererAPI::API::OpenGL: return std::make_shared<OpenGLVertexBuffer>(vertices, size);
        }
    return nullptr;
};

VertexBuffer::~VertexBuffer()
{
};
