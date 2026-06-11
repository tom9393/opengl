#pragma once

#include "pch.h"
#include "Buffer.h"

namespace Engine
{

    class RendererAPI
    {
        public:
        enum class API
        {
            None = 0,
            OpenGL = 1,
            Vulkan = 2
        };

        virtual void Init() = 0;

        virtual void SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height) = 0;
        virtual void ToggleWireframe() = 0;

        virtual void SetClearColor(const glm::vec4 &color) = 0;
        virtual void Clear() = 0;
        virtual void ClearColorOnly() = 0;

        virtual void SetDepthTest(bool enabled) = 0;
        virtual void SetDepthWrite(bool enabled) = 0;
        virtual void SetFaceCulling(bool enabled) = 0;
        virtual void SetAdditiveBlend(bool enabled) = 0;
        virtual void SetAlphaBlend(bool enabled) = 0;

        // Bind a 2D texture (by renderer id) to a sampler slot.
        virtual void BindTexture2D(unsigned int slot, unsigned int rendererID) = 0;
        virtual void BindTextureCube(unsigned int slot, unsigned int rendererID) = 0;
        // Bind a texture of any target type (array, cubemap, ...) to a sampler slot.
        virtual void BindTexture(unsigned int slot, unsigned int rendererID, BufferTextureType type) = 0;

        // Annotate a span of GL commands for debuggers like RenderDoc.
        virtual void PushDebugGroup(const char* name) = 0;
        virtual void PopDebugGroup() = 0;

        // GPU elapsed-time queries (one may be active at a time; they don't nest).
        virtual unsigned int CreateTimerQuery() = 0;
        virtual void DeleteTimerQuery(unsigned int id) = 0;
        virtual void BeginTimerQuery(unsigned int id) = 0;
        virtual void EndTimerQuery() = 0;
        // Non-blocking; returns true and writes the elapsed time when the result
        // is available.
        virtual bool GetTimerQueryResult(unsigned int id, uint64_t& nanoseconds) = 0;

        virtual void DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray) = 0;
        virtual void DrawIndexedInstances(const std::shared_ptr<VertexArray> &vertexArray, unsigned int count) = 0;
        virtual void DrawLines(const std::shared_ptr<VertexArray> &vertexArray, uint32_t count) = 0;
        virtual void DrawPoints(const std::shared_ptr<VertexArray> &vertexArray, uint32_t count) = 0;
        virtual void DrawFullscreenQuad() = 0;
        virtual void DrawCube() = 0;

        static API GetAPI() { return s_API; }

        private:
        static API s_API;
    };

};
