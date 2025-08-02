#include "../include/IndexBuffer.h"
#include "../include/Renderer.h"
#include "../platform/OpenGL/OpenGLIndexBuffer.h"

std::shared_ptr<IndexBuffer> IndexBuffer::Create(uint32_t *indices, uint32_t size)
{
        switch (Renderer::Get()) {
                case RendererAPI::API::None: return nullptr;
                case RendererAPI::API::OpenGL: return std::make_shared<OpenGLIndexBuffer>(indices, size);
        }
    return nullptr;
};

IndexBuffer::~IndexBuffer()
{
};
