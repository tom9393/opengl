#pragma once

#include "RenderPass.h"
#include "Render/RenderGraph.h"
#include "Render/Bloom.h"
#include "Asset/DefaultTextures.h"

namespace Engine {

    // Tonemapping + post effects. Reads the HDR source resource (the particle
    // composite by default, or plain SceneColor in graphs without a ParticlePass),
    // runs the bloom chain, then composites exposure / ACES tonemap / gamma /
    // vignette / chromatic aberration into the final LDR FinalColor target.
    class PostProcessPass : public RenderPass {
    public:
        PostProcessPass(const std::string& name, const std::string& source = "SceneColorParticles")
            : RenderPass(name), m_Source(source) {}

        void Setup(RenderGraph& graph) override {
            m_RenderGraph = &graph;
            SetTargetType(BufferTextureType::Texture2D);
            Output("FinalColor", FramebufferTextureFormat::RGBA8);
            m_SourceInput = Input(m_Source);
            SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        }

        void Execute(RenderContext& ctx, const RenderView& view) override {
            const PostProcessSettings& post = view.Post;

            unsigned int sceneTex = GetInputTextureID(m_SourceInput);
            if (sceneTex == 0) return;

            unsigned int bloomTex = DefaultTextures::GetBlackTexture()->GetID();
            if (post.BloomEnabled) {
                m_Bloom.Resize(view.ViewportSize.x, view.ViewportSize.y);
                bloomTex = m_Bloom.Render(sceneTex, post.BloomThreshold);
            }

            // The bloom sub-renderer bound its own framebuffer; rebind ours.
            GetFramebuffer()->Bind();

            Shared<Shader> shader = ShaderLibrary::Get("PostProcess");
            shader->Bind();
            shader->SetInt("u_SceneColor", 0);
            shader->SetInt("u_BloomColor", 1);
            shader->SetFloat("u_Exposure", post.Exposure);
            shader->SetInt("u_BloomEnabled", post.BloomEnabled ? 1 : 0);
            shader->SetFloat("u_BloomIntensity", post.BloomIntensity);
            shader->SetFloat("u_VignetteStrength", post.VignetteStrength);
            shader->SetFloat("u_ChromaticAberration", post.ChromaticAberration);

            RenderCommand::BindTexture2D(0, sceneTex);
            RenderCommand::BindTexture2D(1, bloomTex);

            Renderer::DrawFullscreenQuad();
        }

    private:
        std::string m_Source;
        InputHandle m_SourceInput = 0;
        Bloom m_Bloom;
    };

}
