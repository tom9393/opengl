#include "../include/VertexArray.h"
#include "../include/Renderer.h"
#include "../include/OpenGLVertexArray.h"

VertexArray* VertexArray::Create()
{
    switch (Renderer::Get()) {
            case RendererAPI::None: return nullptr;
            case RendererAPI::OpenGL: return new OpenGLVertexArray();
        }
    return nullptr;
};

VertexArray::~VertexArray()
{
};
