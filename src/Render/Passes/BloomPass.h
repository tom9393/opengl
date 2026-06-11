#pragma once

#include "RenderPass.h"
#include "Render/RenderGraph.h"

namespace Engine {

    // Progressive dual-filter bloom (Call of Duty / Froyok) expressed as graph
    // passes: a chain of viewport-relative downsample mips ("Bloom.Mip0" at half
    // resolution, each level halving again), then tent-filter upsamples rendered
    // additively in place into the next-larger mip (aliased as "Bloom.UpN").
    // PostProcessPass consumes "Bloom.Up0". Register with AddBloomPasses().

    constexpr int   BloomMipCount     = 6;
    constexpr float BloomFilterRadius = 0.005f;

    class BloomDownsamplePass : public RenderPass {
    public:
        BloomDownsamplePass(const std::string& name, int level, const std::string& source)
            : RenderPass(name), m_Level(level), m_Source(source) {}

        void Setup(RenderGraph& graph) override {
            m_RenderGraph = &graph;
            SetTargetType(BufferTextureType::Texture2D);
            Output("Bloom.Mip" + std::to_string(m_Level), FramebufferTextureFormat::RGBA16F);
            SetSizeScale(1.0f / (float)(1 << (m_Level + 1)));
            m_Input = Input(m_Level == 0 ? m_Source : "Bloom.Mip" + std::to_string(m_Level - 1));

            // The filter quad covers every pixel; no clear needed.
            DisableClear();
            PipelineState state;
            state.depthTest = false;
            state.depthWrite = false;
            SetPipelineState(state);
            SetCondition([](const RenderView& view) { return view.Post.BloomEnabled; });
        }

        void Execute(RenderContext& ctx, const RenderView& view) override {
            if (!m_Shader) m_Shader = ShaderLibrary::Get("BloomDownsample");
            m_Shader->Bind();

            glm::uvec2 srcSize = GetInputSize(m_Input);
            m_Shader->SetVec2("u_SrcResolution", { (float)srcSize.x, (float)srcSize.y });
            m_Shader->SetInt("u_FirstMip", m_Level == 0 ? 1 : 0);
            m_Shader->SetFloat("u_Threshold", view.Post.BloomThreshold);

            BindInput(m_Input, 0);
            Renderer::DrawFullscreenQuad();
        }

    private:
        int m_Level;
        std::string m_Source;
        InputHandle m_Input = 0;
        Shared<Shader> m_Shader;
    };

    class BloomUpsamplePass : public RenderPass {
    public:
        BloomUpsamplePass(const std::string& name, int level)
            : RenderPass(name), m_Level(level) {}

        void Setup(RenderGraph& graph) override {
            m_RenderGraph = &graph;
            SetTargetType(BufferTextureType::Texture2D);
            // Blends additively into this level's downsampled mip in place. The
            // smallest level reads its raw mip; larger levels read the already
            // upsampled-into version below them.
            AliasOutput("Bloom.Up" + std::to_string(m_Level), "Bloom.Mip" + std::to_string(m_Level));
            m_Input = Input(m_Level == BloomMipCount - 2
                                ? "Bloom.Mip" + std::to_string(m_Level + 1)
                                : "Bloom.Up" + std::to_string(m_Level + 1));

            PipelineState state;
            state.depthTest = false;
            state.depthWrite = false;
            state.blend = PassBlend::Additive;
            SetPipelineState(state);
            SetCondition([](const RenderView& view) { return view.Post.BloomEnabled; });
        }

        void Execute(RenderContext& ctx, const RenderView& view) override {
            if (!m_Shader) m_Shader = ShaderLibrary::Get("BloomUpsample");
            m_Shader->Bind();
            m_Shader->SetFloat("u_FilterRadius", BloomFilterRadius);

            BindInput(m_Input, 0);
            Renderer::DrawFullscreenQuad();
        }

    private:
        int m_Level;
        InputHandle m_Input = 0;
        Shared<Shader> m_Shader;
    };

    inline void AddBloomPasses(RenderGraph& graph, const std::string& source)
    {
        for (int i = 0; i < BloomMipCount; i++)
            graph.AddPass<BloomDownsamplePass>("BloomDown" + std::to_string(i), i, source);
        for (int i = BloomMipCount - 2; i >= 0; i--)
            graph.AddPass<BloomUpsamplePass>("BloomUp" + std::to_string(i), i);
    }

}
