#include "../include/IndexBuffer.h"
#include "../include/Renderer.h"
#include "../include/OpenGLIndexBuffer.h"

IndexBuffer* IndexBuffer::Create(uint32_t *indices, uint32_t size)
{
        switch (Renderer::Get()) {
                case RendererAPI::API::None: return nullptr;
                case RendererAPI::API::OpenGL: return new OpenGLIndexBuffer(indices, size);
        }
    return nullptr;
};

IndexBuffer::~IndexBuffer()
{
};
