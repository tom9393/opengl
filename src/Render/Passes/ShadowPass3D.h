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
            SetLayers(MaxShadowCastersPoint);
            SetCondition([](const RenderView& view) {
                return HasFlag(view.Flags, RenderFlags::Shadows) && !view.PointShadowCasters.empty();
            });
        }
        void Execute(RenderContext& ctx, const RenderView& view) override {
            Shared<Shader> pointshadowShader = ShaderLibrary::Get("PointShadowShader");
            pointshadowShader->Bind();

            glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, PointShadowFarPlane);

            size_t casterCount = std::min(view.PointShadowCasters.size(), (size_t)MaxShadowCastersPoint);
            for (size_t layer = 0; layer < casterCount; ++layer) {
                auto& caster = view.PointShadowCasters[layer];
                const glm::vec3& lightPos = caster.Position;

                static constexpr glm::vec3 dirs[6] = { { 1,0,0}, {-1,0,0}, {0, 1,0}, {0,-1,0}, {0,0, 1}, {0,0,-1} };
                static constexpr glm::vec3 ups[6] = { {0,-1,0}, {0,-1,0}, {0,0, 1}, {0,0,-1}, {0,-1,0}, {0,-1,0} };

                std::array<glm::mat4, 6> shadowTransforms;
                for (int i = 0; i < 6; ++i) shadowTransforms[i] = shadowProj * glm::lookAt(lightPos, lightPos + dirs[i], ups[i]);

                // Per-light face matrices go through the frame's pool as a UBO
                // (binding 2; 0/1 are the camera and light blocks).
                Allocation alloc = ctx.InstancePool->Allocate(shadowTransforms.data(), sizeof(shadowTransforms));
                if (alloc.Size == 0) continue;
                ctx.InstancePool->BindAsUBO(2, alloc.Offset, alloc.Size);

                pointshadowShader->SetFloat("u_FarPlane", PointShadowFarPlane);
                pointshadowShader->SetVec3("u_LightPos", lightPos);
                pointshadowShader->SetInt("u_BaseLayer", layer);

                DrawBatched(ctx, caster.RenderItems, false);
            }
        }
    };

}
