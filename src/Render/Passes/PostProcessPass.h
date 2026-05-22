#pragma once

#include "RenderPass.h"
#include "Render/RenderGraph.h"
#include "Render/Bloom.h"
#include "Asset/DefaultTextures.h"

namespace Engine {

    // Tonemapping + post effects. Reads the HDR SceneColor, runs the bloom chain,
    // then composites exposure / ACES tonemap / gamma / vignette / chromatic
    // aberration into the final LDR FinalColor target.
    class PostProcessPass : public RenderPass {
    public:
        PostProcessPass(const std::string& name) : RenderPass(name) {}

        void Setup(RenderGraph& graph) override {
            m_RenderGraph = &graph;
            SetTargetType(BufferTextureType::Texture2D);
            Output("FinalColor", FramebufferTextureFormat::RGBA8);
            Input("SceneColor");
            SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        }

        void Execute(RenderContext& ctx, const RenderView& view) override {
            const PostProcessSettings& post = view.Post;

            Shared<FramebufferHandle> sceneFb = m_RenderGraph->GetResource("SceneColor");
            if (!sceneFb) return;
            unsigned int sceneTex = sceneFb->GetColorAttachmentRendererID(0);

            unsigned int bloomTex = DefaultTextures::GetBlackTexture()->GetID();
            if (post.BloomEnabled) {
                m_Bloom.Resize(view.ViewportSize.x, view.ViewportSize.y);
                bloomTex = m_Bloom.Render(sceneTex, post.BloomThreshold);
            }

            // The bloom sub-renderer bound its own framebuffer; rebind ours.
            m_RenderGraph->GetPassFramebuffer(GetName())->GetFramebuffer()->Bind();

            Shared<Shader> shader = ShaderLibrary::Get("PostProcess");
            shader->Bind();
            shader->SetInt("u_SceneColor", 0);
            shader->SetInt("u_BloomColor", 1);
            shader->SetFloat("u_Exposure", post.Exposure);
            shader->SetInt("u_BloomEnabled", post.BloomEnabled ? 1 : 0);
            shader->SetFloat("u_BloomIntensity", post.BloomIntensity);
            shader->SetFloat("u_VignetteStrength", post.VignetteStrength);
            shader->SetFloat("u_ChromaticAberration", post.ChromaticAberration);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sceneTex);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, bloomTex);

            Renderer::DrawFullscreenQuad();
        }

    private:
        Bloom m_Bloom;
    };

}
