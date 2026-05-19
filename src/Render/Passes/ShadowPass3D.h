#pragma once

#include "RenderPass.h"
#include "Render/RenderGraph.h"

namespace Engine {

    class ShadowPass3D : public RenderPass {
    public:
        ShadowPass3D(const std::string& name) : RenderPass(name) {}

        void Setup(RenderGraph& graph) override {
            m_RenderGraph = &graph;
            SetTargetType(BufferTextureType::Cubemap_Array);
            Output("PointShadowMap", FramebufferTextureFormat::DEPTH24STENCIL8);
            SetViewportSize(128, 128);
            SetCondition([](const RenderView& view) {
                return HasFlag(view.Flags, RenderFlags::Shadows) && !view.PointShadowCasters.empty();
            });
        }
        void Execute(RenderContext& ctx, const RenderView& view) override {
            Shared<Shader> pointshadowShader = ShaderLibrary::Get("PointShadowShader");
            pointshadowShader->Bind();

            glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, m_PointLightFarPlane);

            for (size_t layer = 0; layer < view.PointShadowCasters.size(); ++layer) {
                auto& caster = view.PointShadowCasters[layer];
                const glm::vec3& lightPos = caster.Position;

                static constexpr glm::vec3 dirs[6] = { { 1,0,0}, {-1,0,0}, {0, 1,0}, {0,-1,0}, {0,0, 1}, {0,0,-1} };
                static constexpr glm::vec3 ups[6] = { {0,-1,0}, {0,-1,0}, {0,0, 1}, {0,0,-1}, {0,-1,0}, {0,-1,0} };

                std::array<glm::mat4, 6> shadowTransforms;
                for (int i = 0; i < 6; ++i) shadowTransforms[i] = shadowProj * glm::lookAt(lightPos, lightPos + dirs[i], ups[i]);
                for (int i = 0; i < 6; ++i) pointshadowShader->SetMat4("u_ShadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);

                pointshadowShader->SetFloat("u_FarPlane", m_PointLightFarPlane);
                pointshadowShader->SetVec3("u_LightPos", lightPos);
                pointshadowShader->SetInt("u_BaseLayer", layer);

                DrawBatched(ctx, caster.RenderItems);
            }
        }

    private:
        void DrawBatched(RenderContext& ctx, const auto& renderItems) {
            size_t i = 0;
            while (i < renderItems.size()) {
                size_t batchSize = 0;
                while (i + batchSize < renderItems.size() && renderItems[i + batchSize].TargetMesh == renderItems[i].TargetMesh)
                    batchSize++;

                m_Transforms.clear();
                m_Transforms.reserve(batchSize);
                for (size_t j = 0; j < batchSize; j++)
                    m_Transforms.emplace_back(renderItems[i + j].Transform);

                Allocation alloc = ctx.InstancePool->Allocate(m_Transforms.data(), m_Transforms.size() * sizeof(glm::mat4));
                ctx.InstancePool->BindAsSSBO(0, alloc.Offset, alloc.Size);
                Renderer::SubmitInstancedMesh(renderItems[i].TargetMesh, nullptr, m_Transforms.size());
                i += batchSize;
            }
        }

        float m_PointLightFarPlane = 35.0f;
        std::vector<glm::mat4> m_Transforms;
    };

}
