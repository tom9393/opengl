#include "../include/RenderCommand.h"
#include "../platform/OpenGL/OpenGLRendererAPI.h"

RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
