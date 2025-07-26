#include "../include/IndexBuffer.h"
#include "../include/Renderer.h"
#include "../include/OpenGLIndexBuffer.h"

IndexBuffer* IndexBuffer::Create(uint32_t *indices, uint32_t size)
{
        switch (Renderer::Get()) {
        case RendererAPI::None: return nullptr;
        case RendererAPI::OpenGL: return new OpenGLIndexBuffer(indices, size);
        }
    return nullptr;
};

IndexBuffer::~IndexBuffer()
{
};
