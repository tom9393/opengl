#pragma once

#include "RenderPass.h"
#include "Render/RenderGraph.h"
#include "Asset/DefaultTextures.h"

namespace Engine {

    // Tonemapping + post effects. Reads the HDR source resource (the particle
    // composite by default, or plain SceneColor in graphs without a ParticlePass)
    // plus the bloom chain's largest mip, then composites exposure / ACES tonemap /
    // gamma / vignette / chromatic aberration into the final LDR FinalColor target.
    // Graphs without bloom passes construct with withBloom = false.
    class PostProcessPass : public RenderPass {
    public:
        PostProcessPass(const std::string& name, const std::string& source = "SceneColorParticles",
                        bool withBloom = true)
            : RenderPass(name), m_Source(source), m_WithBloom(withBloom) {}

        void Setup(RenderGraph& graph) override {
            m_RenderGraph = &graph;
            SetTargetType(BufferTextureType::Texture2D);
            Output("FinalColor", FramebufferTextureFormat::RGBA8);
            m_SourceInput = Input(m_Source);
            if (m_WithBloom) m_BloomInput = Input("Bloom.Up0");

            // The composite quad covers every pixel; no clear needed.
            DisableClear();
            PipelineState state;
            state.depthTest = false;
            state.depthWrite = false;
            SetPipelineState(state);
        }

        void Execute(RenderContext& ctx, const RenderView& view) override {
            const PostProcessSettings& post = view.Post;

            unsigned int sceneTex = GetInputTextureID(m_SourceInput);
            if (sceneTex == 0) return;

            bool bloomEnabled = m_WithBloom && post.BloomEnabled;
            unsigned int bloomTex = bloomEnabled ? GetInputTextureID(m_BloomInput)
                                                 : DefaultTextures::GetBlackTexture()->GetID();

            if (!m_Shader) m_Shader = ShaderLibrary::Get("PostProcess");
            m_Shader->Bind();
            m_Shader->SetFloat("u_Exposure", post.Exposure);
            m_Shader->SetInt("u_BloomEnabled", bloomEnabled ? 1 : 0);
            m_Shader->SetFloat("u_BloomIntensity", post.BloomIntensity);
            m_Shader->SetFloat("u_VignetteStrength", post.VignetteStrength);
            m_Shader->SetFloat("u_ChromaticAberration", post.ChromaticAberration);

            RenderCommand::BindTexture2D(0, sceneTex);
            RenderCommand::BindTexture2D(1, bloomTex);

            Renderer::DrawFullscreenQuad();
        }

    private:
        std::string m_Source;
        bool m_WithBloom;
        InputHandle m_SourceInput = 0;
        InputHandle m_BloomInput = 0;
        Shared<Shader> m_Shader;
    };

}
