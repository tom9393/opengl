#pragma once

#include "../../include/RendererAPI.h"

class OpenGLRendererAPI : public RendererAPI
{
public:
    void Init() override;
    void SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height) override;
    void SetClearColor(const glm::vec4 &color) override;
    void Clear() override;

    void DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray) override;
    void DrawIndexedInstances(const std::shared_ptr<VertexArray> &vertexArray, unsigned int count) override;
    void ToggleWireframe() override;

    private:
    bool m_Wireframe = false;
};
