#pragma once

#include "Buffer.h"
#include "RendererAPI.h"

namespace Engine
{

    class RenderCommand
    {
        public:

        inline static void Init()
        {
            s_RendererAPI->Init();
        }

        inline static void SetClearColor(const glm::vec4 &color)
        {
            s_RendererAPI->SetClearColor(color);
        }
        inline static void Clear()
        {
            s_RendererAPI->Clear();
        }
        inline static void ClearColorOnly()
        {
            s_RendererAPI->ClearColorOnly();
        }

        inline static void SetDepthTest(bool enabled)
        {
            s_RendererAPI->SetDepthTest(enabled);
        }
        inline static void SetDepthWrite(bool enabled)
        {
            s_RendererAPI->SetDepthWrite(enabled);
        }
        inline static void SetFaceCulling(bool enabled)
        {
            s_RendererAPI->SetFaceCulling(enabled);
        }
        inline static void SetAdditiveBlend(bool enabled)
        {
            s_RendererAPI->SetAdditiveBlend(enabled);
        }
        inline static void SetAlphaBlend(bool enabled)
        {
            s_RendererAPI->SetAlphaBlend(enabled);
        }

        inline static void BindTexture2D(unsigned int slot, unsigned int rendererID)
        {
            s_RendererAPI->BindTexture2D(slot, rendererID);
        }

        inline static void BindTextureCube(unsigned int slot, unsigned int rendererID)
        {
            s_RendererAPI->BindTextureCube(slot, rendererID);
        }

        inline static void BindTexture(unsigned int slot, unsigned int rendererID, BufferTextureType type)
        {
            s_RendererAPI->BindTexture(slot, rendererID, type);
        }

        inline static void PushDebugGroup(const char* name)
        {
            s_RendererAPI->PushDebugGroup(name);
        }

        inline static void PopDebugGroup()
        {
            s_RendererAPI->PopDebugGroup();
        }

        inline static unsigned int CreateTimerQuery()
        {
            return s_RendererAPI->CreateTimerQuery();
        }

        inline static void DeleteTimerQuery(unsigned int id)
        {
            s_RendererAPI->DeleteTimerQuery(id);
        }

        inline static void BeginTimerQuery(unsigned int id)
        {
            s_RendererAPI->BeginTimerQuery(id);
        }

        inline static void EndTimerQuery()
        {
            s_RendererAPI->EndTimerQuery();
        }

        inline static bool GetTimerQueryResult(unsigned int id, uint64_t& nanoseconds)
        {
            return s_RendererAPI->GetTimerQueryResult(id, nanoseconds);
        }

        inline static void DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray)
        {
            s_RendererAPI->DrawIndexed(vertexArray);
        }

        inline static void DrawIndexedInstances(const std::shared_ptr<VertexArray> &vertexArray, unsigned int count)
        {
            s_RendererAPI->DrawIndexedInstances(vertexArray, count);
        }

        inline static void DrawLines(Shared<VertexArray> &lines, uint32_t count)
        {
            s_RendererAPI->DrawLines(lines, count);
        }

        inline static void DrawPoints(Shared<VertexArray> &points, uint32_t count)
        {
            s_RendererAPI->DrawPoints(points, count);
        }

        inline static void DrawFullscreenQuad()
        {
            s_RendererAPI->DrawFullscreenQuad();
        }

        inline static void DrawCube()
        {
            s_RendererAPI->DrawCube();
        }

        inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
        {
            s_RendererAPI->SetViewport(x, y, width, height);
        }

        inline static void ToggleWireframe()
        {
            s_RendererAPI->ToggleWireframe();
        }

        private:
        static RendererAPI* s_RendererAPI;
    };

};
