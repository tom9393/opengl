#pragma once

#include "Render/RendererAPI.h"

namespace Engine
{

    class OpenGLRendererAPI : public RendererAPI
    {
    public:
        void Init() override;
        void SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height) override;
        void SetClearColor(const glm::vec4 &color) override;
        void Clear() override;

        void DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray) override;
        void DrawIndexedInstances(const std::shared_ptr<VertexArray> &vertexArray, unsigned int count) override;
        void DrawLines(const std::shared_ptr<VertexArray> &lines, uint32_t size) override;
        void DrawPoints(const std::shared_ptr<VertexArray> &points, uint32_t size) override;
        void DrawFullscreenQuad() override;

        void ToggleWireframe() override;

        private:
        bool m_Wireframe = false;
    };

};
