#pragma once

#include "RenderCommand.h"
#include "RendererAPI.h"

class Renderer
{
    public:

    static void BeginScene();
    static void EndScene();

    static void Submit(const std::shared_ptr<VertexArray> &vertexArray);

    static RendererAPI::API Get() { return RendererAPI::GetAPI(); }

};
