#pragma once

#include "RenderPass.h"
#include "Render/RenderGraph.h"

namespace Engine {

    class GeometryPass : public RenderPass {
    public:
        GeometryPass(const std::string& name) : RenderPass(name) {}

        void Setup(RenderGraph& graph) override {
            m_RenderGraph = &graph;
            SetTargetType(BufferTextureType::Texture2D);
            Output("GBuffer.Position", FramebufferTextureFormat::RGBA16F);
            Output("GBuffer.Normal",   FramebufferTextureFormat::RGBA16F);
            Output("GBuffer.Albedo",   FramebufferTextureFormat::RGBA8);
            Output("GBuffer.PBR",      FramebufferTextureFormat::RGBA8);
            unsigned int entityIdx = Output("GBuffer.EntityID", FramebufferTextureFormat::RED_INTEGER);
            Output("GBuffer.Depth",    FramebufferTextureFormat::DEPTH24STENCIL8);
            SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
            SetClearInt(entityIdx, -1);
        }
        void Execute(RenderContext& ctx, const RenderView& view) override {
            Shared<Shader> gBufferShader = ShaderLibrary::Get("GBufferShader");
            gBufferShader->Bind();

            size_t i = 0;
            while (i < view.OpaqueItems.size()) {
                auto& base = view.OpaqueItems[i];
                size_t batchSize = 0;

                while (i + batchSize < view.OpaqueItems.size() &&
                    view.OpaqueItems[i + batchSize].TargetMesh == base.TargetMesh &&
                    view.OpaqueItems[i + batchSize].TargetMaterial == base.TargetMaterial)
                {
                    batchSize++;
                }

                m_Transforms.clear();
                m_Transforms.reserve(batchSize);
                for (size_t j = 0; j < batchSize; j++)
                    m_Transforms.emplace_back(view.OpaqueItems[i + j].Transform);

                size_t dataSize = m_Transforms.size() * sizeof(glm::mat4);

                // -- THE MAGIC HAPPENS HERE --
                // 1. Get memory from the shared pool
                Allocation alloc = ctx.InstancePool->Allocate(m_Transforms.data(), dataSize);

                // 2. Bind the exact offset to SSBO slot 0
                ctx.InstancePool->BindAsSSBO(0, alloc.Offset, alloc.Size);

                Renderer::SubmitInstancedMesh(base.TargetMesh, base.TargetMaterial, m_Transforms.size());
                i += batchSize;
            }
        }

        std::vector<glm::mat4> m_Transforms;
    };

}
