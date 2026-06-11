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
        void ClearColorOnly() override;

        void SetDepthTest(bool enabled) override;
        void SetDepthWrite(bool enabled) override;
        void SetFaceCulling(bool enabled) override;
        void SetAdditiveBlend(bool enabled) override;
        void SetAlphaBlend(bool enabled) override;

        void BindTexture2D(unsigned int slot, unsigned int rendererID) override;
        void BindTextureCube(unsigned int slot, unsigned int rendererID) override;
        void BindTexture(unsigned int slot, unsigned int rendererID, BufferTextureType type) override;

        void PushDebugGroup(const char* name) override;
        void PopDebugGroup() override;

        unsigned int CreateTimerQuery() override;
        void DeleteTimerQuery(unsigned int id) override;
        void BeginTimerQuery(unsigned int id) override;
        void EndTimerQuery() override;
        bool GetTimerQueryResult(unsigned int id, uint64_t& nanoseconds) override;

        void DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray) override;
        void DrawIndexedInstances(const std::shared_ptr<VertexArray> &vertexArray, unsigned int count) override;
        void DrawLines(const std::shared_ptr<VertexArray> &lines, uint32_t size) override;
        void DrawPoints(const std::shared_ptr<VertexArray> &points, uint32_t size) override;
        void DrawFullscreenQuad() override;
        void DrawCube() override;

        void ToggleWireframe() override;

        private:
        bool m_Wireframe = false;
    };

};
