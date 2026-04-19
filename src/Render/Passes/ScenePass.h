#pragma once

#include "RenderPass.h"
#include "../RenderGraph.h"

namespace Engine {

    class ScenePass : public RenderPass {
    public:
        ScenePass(const std::string& name) : RenderPass(name) {}

        void Setup(RenderGraph& graph) override {
            m_RenderGraph = &graph;
            SetAttachments({ FramebufferTextureFormat::RGBA8 }, BufferTextureType::Texture2D );
            SetClearColor({ 0.2f, 0.2f, 0.2f, 1.0f });
        }
        void Execute(RenderContext& ctx, const RenderView& view) override {
            Shared<Shader> deferredShader = ShaderLibrary::Get("DeferredLighting");
            deferredShader->Bind();

            // Note: In your actual engine, get these texture IDs from the G-Buffer Pass Framebuffer
            deferredShader->SetInt("gPosition", 0);
            deferredShader->SetInt("gNormal", 1);
            deferredShader->SetInt("gAlbedo", 2);
            deferredShader->SetInt("gPBR", 3);
            
            // Set Shadow Maps
            deferredShader->SetInt("u_DirSpotShadowMaps", 4);    // Updated to array
            deferredShader->SetInt("u_PointShadowCubemaps", 5);  // Updated to array
            deferredShader->SetFloat("u_FarPlane", m_PointLightFarPlane);

            auto gBufferFB = m_RenderGraph->GetPassFramebuffer("GeometryPass");
            auto shadow2DFB = m_RenderGraph->GetPassFramebuffer("ShadowPass2D");
            auto shadowCubeFB = m_RenderGraph->GetPassFramebuffer("PointShadowPass");

            if (gBufferFB)
            {
                gBufferFB->AttachToTextureSlot(0, 0);
                gBufferFB->AttachToTextureSlot(1, 1);
                gBufferFB->AttachToTextureSlot(2, 2);
                gBufferFB->AttachToTextureSlot(3, 3);
            }

            if (shadow2DFB)
            {
                shadow2DFB->AttachToTextureSlotDepth(4);
            }

            if (shadowCubeFB)
            {
                shadowCubeFB->AttachToTextureSlotDepth(5);
            }

            Renderer::DrawFullscreenQuad(); 
            
            PhysicsEngine::RenderDebug(ctx.ViewProjection);
        }

        float m_PointLightFarPlane = 35.0f;
    };

}
