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
            SetCondition([](const RenderView& view) {
                return !view.DirectionalShadowCasters.empty();
            });
        }
        void Execute(RenderContext& ctx, const RenderView& view) override {
            Shared<Shader> shadowShader = ShaderLibrary::Get("ShadowShader");
            shadowShader->Bind();

            auto fb = m_RenderGraph->GetPassFramebuffer(GetName());

            for (size_t layer = 0; layer < view.DirectionalShadowCasters.size(); ++layer) {
                auto& caster = view.DirectionalShadowCasters[layer];
                fb->GetFramebuffer()->AttachLayer(layer);

                shadowShader->SetMat4("u_LightSpaceMatrix", caster.LightSpaceMatrix);

                size_t i = 0;
                while (i < caster.RenderItems.size()) {
                    size_t batchSize = 0;
                    while (i + batchSize < caster.RenderItems.size() && caster.RenderItems[i + batchSize].TargetMesh == caster.RenderItems[i].TargetMesh) {
                        batchSize++;
                    }

                    m_Transforms.clear();
                    m_Transforms.reserve(batchSize);
                    for (size_t j = 0; j < batchSize; j++)
                        m_Transforms.emplace_back(caster.RenderItems[i + j].Transform);


                    size_t dataSize = m_Transforms.size() * sizeof(glm::mat4);

                    // -- THE MAGIC HAPPENS HERE --
                    // 1. Get memory from the shared pool
                    Allocation alloc = ctx.InstancePool->Allocate(m_Transforms.data(), dataSize);

                    // 2. Bind the exact offset to SSBO slot 0
                    ctx.InstancePool->BindAsSSBO(0, alloc.Offset, alloc.Size);

                    Renderer::SubmitInstancedMesh(caster.RenderItems[i].TargetMesh, nullptr, m_Transforms.size());
                    i += batchSize;
                }
            }
        }

        float m_PointLightFarPlane = 35.0f;
        std::vector<glm::mat4> m_Transforms;
    };

}
