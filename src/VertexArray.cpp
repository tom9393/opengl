#include "../include/VertexArray.h"
#include "../include/Renderer.h"
#include "../include/OpenGLVertexArray.h"

VertexArray* VertexArray::Create()
{
    switch (Renderer::Get()) {
        case RendererAPI::API::None: return nullptr;
        case RendererAPI::API::OpenGL: return new OpenGLVertexArray();
        }
    return nullptr;
};

VertexArray::~VertexArray()
{
};
