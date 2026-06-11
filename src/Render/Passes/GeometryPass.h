#pragma once

#include "RenderPass.h"
#include "Render/RenderGraph.h"

namespace Engine {

    class GeometryPass : public RenderPass {
    public:
        GeometryPass(const std::string& name) : RenderPass(name) {}

        void Setup(RenderGraph& graph) override {
            m_RenderGraph = &graph;
            SetTargetType(BufferTextureType::Texture2D);
            // World position is not stored; the lighting pass reconstructs it from depth.
            Output("GBuffer.Normal",   FramebufferTextureFormat::RGBA16F);
            Output("GBuffer.Albedo",   FramebufferTextureFormat::RGBA8);
            Output("GBuffer.PBR",      FramebufferTextureFormat::RGBA8);
            Output("GBuffer.Depth",    FramebufferTextureFormat::DEPTH24STENCIL8);
            SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        }
        void Execute(RenderContext& ctx, const RenderView& view) override {
            Shared<Shader> gBufferShader = ShaderLibrary::Get("GBufferShader");
            gBufferShader->Bind();

            DrawBatched(ctx, view.OpaqueItems, true);
        }
    };

}
