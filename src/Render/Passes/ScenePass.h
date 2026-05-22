#pragma once

#include "RenderPass.h"
#include "Render/RenderGraph.h"
#include "Render/EnvironmentMap.h"

namespace Engine {

    class ScenePass : public RenderPass {
    public:
        ScenePass(const std::string& name) : RenderPass(name) {}

        void Setup(RenderGraph& graph) override {
            m_RenderGraph = &graph;
            SetTargetType(BufferTextureType::Texture2D);
            // HDR target: lighting outputs linear radiance; tonemap is in the post pass.
            Output("SceneColor", FramebufferTextureFormat::RGBA16F);
            Input("GBuffer.Position");
            Input("GBuffer.Normal");
            Input("GBuffer.Albedo");
            Input("GBuffer.PBR");
            Input("ShadowMap2D");
            Input("PointShadowMap");
            SetClearColor({ 0.2f, 0.2f, 0.2f, 1.0f });
        }
        void Execute(RenderContext& ctx, const RenderView& view) override {
            Shared<Shader> deferredShader = ShaderLibrary::Get("DeferredLighting");
            deferredShader->Bind();

            deferredShader->SetInt("gPosition", 0);
            deferredShader->SetInt("gNormal", 1);
            deferredShader->SetInt("gAlbedo", 2);
            deferredShader->SetInt("gPBR", 3);

            deferredShader->SetInt("u_DirSpotShadowMaps", 4);
            deferredShader->SetInt("u_PointShadowCubemaps", 5);
            deferredShader->SetFloat("u_FarPlane", m_PointLightFarPlane);

            m_RenderGraph->BindInput("GBuffer.Position", 0);
            m_RenderGraph->BindInput("GBuffer.Normal",   1);
            m_RenderGraph->BindInput("GBuffer.Albedo",   2);
            m_RenderGraph->BindInput("GBuffer.PBR",      3);

            if (m_RenderGraph->HasInput("ShadowMap2D"))
                m_RenderGraph->BindInput("ShadowMap2D", 4);
            if (m_RenderGraph->HasInput("PointShadowMap"))
                m_RenderGraph->BindInput("PointShadowMap", 5);

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
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_CUBE_MAP, env.Valid ? env.Irradiance : fallbackCube);
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_CUBE_MAP, env.Valid ? env.Prefilter : fallbackCube);
            glActiveTexture(GL_TEXTURE8);
            glBindTexture(GL_TEXTURE_2D, EnvironmentMap::GetBRDFLUT());
            glActiveTexture(GL_TEXTURE9);
            glBindTexture(GL_TEXTURE_CUBE_MAP, env.Valid ? env.Skybox : fallbackCube);

            Renderer::DrawFullscreenQuad();

            PhysicsEngine::RenderDebug(ctx.ViewProjection);
        }

        float m_PointLightFarPlane = 35.0f;
    };

}
