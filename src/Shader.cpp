#include "../include/Shader.h"
#include "../include/Renderer.h"
#include "../platform/OpenGL/OpenGLShader.h"

Shader* Shader::Create(const char* vertexSrc, const char* fragmentSrc)
{
    switch (Renderer::Get()) {
        case RendererAPI::API::None: return nullptr;
        case RendererAPI::API::OpenGL: return new OpenGLShader(vertexSrc, fragmentSrc);
        }
    return nullptr;
};
