#pragma once

#include "RenderPass.h"
#include "Render/RenderGraph.h"

namespace Engine {

    class ShadowPass2D : public RenderPass {
    public:
        ShadowPass2D(const std::string& name) : RenderPass(name) {}

        void Setup(RenderGraph& graph) override {
            m_RenderGraph = &graph;
            SetTargetType(BufferTextureType::Texture2D_Array);
            Output("ShadowMap2D", FramebufferTextureFormat::DEPTH24STENCIL8);
            SetViewportSize(1024, 1024);
            SetLayers(MaxShadowCasters2D);
            SetCondition([](const RenderView& view) {
                return HasFlag(view.Flags, RenderFlags::Shadows) &&
                       (!view.DirectionalShadowCasters.empty() || !view.ConeShadowCasters.empty());
            });
        }
        void Execute(RenderContext& ctx, const RenderView& view) override {
            Shared<Shader> shadowShader = ShaderLibrary::Get("ShadowShader");
            shadowShader->Bind();

            auto fb = m_RenderGraph->GetPassFramebuffer(GetName());

            size_t layer = 0;
            auto renderCaster = [&](const auto& caster) {
                if (layer >= (size_t)MaxShadowCasters2D) return;
                fb->GetFramebuffer()->AttachLayer(layer++);
                shadowShader->SetMat4("u_LightSpaceMatrix", caster.LightSpaceMatrix);

                DrawBatched(ctx, caster.RenderItems, false);
            };

            for (auto& caster : view.DirectionalShadowCasters)
                renderCaster(caster);

            for (auto& caster : view.ConeShadowCasters)
                renderCaster(caster);
        }
    };

}
