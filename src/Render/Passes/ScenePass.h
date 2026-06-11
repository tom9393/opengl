#pragma once

#include "RenderPass.h"
#include "Render/RenderGraph.h"
#include "Render/EnvironmentMap.h"
#include "Physics/PhysicsEngine.h"

namespace Engine {

    class ScenePass : public RenderPass {
    public:
        ScenePass(const std::string& name) : RenderPass(name) {}

        void Setup(RenderGraph& graph) override {
            m_RenderGraph = &graph;
            SetTargetType(BufferTextureType::Texture2D);
            // HDR target: lighting outputs linear radiance; tonemap is in the post pass.
            Output("SceneColor", FramebufferTextureFormat::RGBA16F);
            m_Normal = Input("GBuffer.Normal");
            m_Albedo = Input("GBuffer.Albedo");
            m_PBR    = Input("GBuffer.PBR");
            // Sampled for world-position reconstruction, and attached (read-only) so
            // forward geometry after the lighting quad is depth tested against the scene.
            m_Depth  = SetDepthInput("GBuffer.Depth");
            m_Shadow2D    = Input("ShadowMap2D");
            m_ShadowPoint = Input("PointShadowMap");
            SetClearColor({ 0.2f, 0.2f, 0.2f, 1.0f });
        }
        void Execute(RenderContext& ctx, const RenderView& view) override {
            Shared<Shader> deferredShader = ShaderLibrary::Get("DeferredLighting");
            deferredShader->Bind();

            deferredShader->SetInt("gNormal", 0);
            deferredShader->SetInt("gAlbedo", 1);
            deferredShader->SetInt("gPBR",    2);
            deferredShader->SetInt("gDepth",  3);

            deferredShader->SetInt("u_DirSpotShadowMaps", 4);
            deferredShader->SetInt("u_PointShadowCubemaps", 5);
            deferredShader->SetFloat("u_FarPlane", PointShadowFarPlane);

            BindInput(m_Normal, 0);
            BindInput(m_Albedo, 1);
            BindInput(m_PBR,    2);
            BindInput(m_Depth,  3);

            // Always bound (type-correct even when the shadow passes were skipped this
            // frame); lights only carry shadow indices when a caster was extracted, so
            // stale contents are never sampled.
            BindInput(m_Shadow2D,    4);
            BindInput(m_ShadowPoint, 5);

            // --- Image-based lighting ---
            const EnvironmentRenderData& env = view.Environment;

            deferredShader->SetInt("u_IrradianceMap", 6);
            deferredShader->SetInt("u_PrefilterMap",  7);
            deferredShader->SetInt("u_BrdfLUT",       8);
            deferredShader->SetInt("u_Skybox",        9);
            deferredShader->SetBool("u_HasEnvironment", env.Valid);
            deferredShader->SetFloat("u_EnvIntensity", env.Intensity);
            deferredShader->SetFloat("u_PrefilterMaxLod", (float)env.PrefilterMips - 1.0f);
            deferredShader->SetMat4("u_InverseViewProjection", glm::inverse(ctx.ViewProjection));

            unsigned int fallbackCube = EnvironmentMap::GetFallbackCubemap();
            RenderCommand::BindTextureCube(6, env.Valid ? env.Irradiance : fallbackCube);
            RenderCommand::BindTextureCube(7, env.Valid ? env.Prefilter : fallbackCube);
            RenderCommand::BindTexture2D(8, EnvironmentMap::GetBRDFLUT());
            RenderCommand::BindTextureCube(9, env.Valid ? env.Skybox : fallbackCube);

            // The lighting quad shades every pixel; don't test it against the
            // attached G-buffer depth.
            RenderCommand::SetDepthTest(false);
            Renderer::DrawFullscreenQuad();
            RenderCommand::SetDepthTest(true);

            // Debug geometry draws after lighting, depth tested against the scene
            // (depth writes stay masked by the graph).
            PhysicsEngine::RenderDebug(ctx.ViewProjection);
        }

    private:
        InputHandle m_Normal = 0, m_Albedo = 0, m_PBR = 0, m_Depth = 0;
        InputHandle m_Shadow2D = 0, m_ShadowPoint = 0;
    };

}
