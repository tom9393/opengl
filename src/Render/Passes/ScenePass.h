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

            // The lighting quad shades every pixel, so the baseline skips the
            // depth test; debug geometry re-enables it mid-pass.
            PipelineState state;
            state.depthTest = false;
            SetPipelineState(state);
        }
        void Execute(RenderContext& ctx, const RenderView& view) override {
            if (!m_Shader) m_Shader = ShaderLibrary::Get("DeferredLighting");
            m_Shader->Bind();

            m_Shader->SetFloat("u_FarPlane", PointShadowFarPlane);

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

            m_Shader->SetBool("u_HasEnvironment", env.Valid);
            m_Shader->SetFloat("u_EnvIntensity", env.Intensity);
            m_Shader->SetFloat("u_PrefilterMaxLod", (float)env.PrefilterMips - 1.0f);
            m_Shader->SetMat4("u_InverseViewProjection", glm::inverse(ctx.ViewProjection));

            unsigned int fallbackCube = EnvironmentMap::GetFallbackCubemap();
            RenderCommand::BindTextureCube(6, env.Valid ? env.Irradiance : fallbackCube);
            RenderCommand::BindTextureCube(7, env.Valid ? env.Prefilter : fallbackCube);
            RenderCommand::BindTexture2D(8, EnvironmentMap::GetBRDFLUT());
            RenderCommand::BindTextureCube(9, env.Valid ? env.Skybox : fallbackCube);

            Renderer::DrawFullscreenQuad();

            // Debug geometry draws after lighting, depth tested against the scene
            // (depth writes stay masked by the graph).
            RenderCommand::SetDepthTest(true);
            PhysicsEngine::RenderDebug(ctx.ViewProjection);
        }

    private:
        InputHandle m_Normal = 0, m_Albedo = 0, m_PBR = 0, m_Depth = 0;
        InputHandle m_Shadow2D = 0, m_ShadowPoint = 0;
        Shared<Shader> m_Shader;
    };

}
