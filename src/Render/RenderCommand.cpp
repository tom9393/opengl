#include "RenderCommand.h"
#include "OpenGL/OpenGLRendererAPI.h"

namespace Engine
{

    RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;

};
