#include "../include/VertexBuffer.h"
#include "../include/Renderer.h"
#include "../include/OpenGLVertexBuffer.h"

VertexBuffer* VertexBuffer::Create(float *vertices, uint32_t size)
{
        switch (Renderer::Get()) {
        case RendererAPI::None: return nullptr;
        case RendererAPI::OpenGL: return new OpenGLVertexBuffer(vertices, size);
        }
    return nullptr;
};

VertexBuffer::~VertexBuffer()
{
};
