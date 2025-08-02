#include "../include/VertexArray.h"
#include "../include/Renderer.h"
#include "../platform/OpenGL/OpenGLVertexArray.h"

std::shared_ptr<VertexArray> VertexArray::Create()
{
    switch (Renderer::Get()) {
        case RendererAPI::API::None: return nullptr;
        case RendererAPI::API::OpenGL: return std::make_shared<OpenGLVertexArray>();
        }
    return nullptr;
};

VertexArray::~VertexArray()
{
};
