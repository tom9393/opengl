#include "../include/RenderCommand.h"
#include "../include/OpenGLRendererAPI.h"

RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
