#pragma once

#include "RenderPass.h"
#include "Render/RenderGraph.h"
#include "Asset/Texture.h"

namespace Engine {

    // Composites SceneColor into its own target, then blends the extracted
    // particle batches on top as camera-facing instanced quads. The target has
    // no depth attachment; occlusion is a manual G-buffer depth test in
    // particle.frag.
    class ParticlePass : public RenderPass {
    public:
        ParticlePass(const std::string& name) : RenderPass(name) {}

        void Setup(RenderGraph& graph) override {
            m_RenderGraph = &graph;
            SetTargetType(BufferTextureType::Texture2D);
            Output("SceneColorParticles", FramebufferTextureFormat::RGBA16F);
            Input("SceneColor");
            Input("GBuffer.Depth");
        }

        void Execute(RenderContext& ctx, const RenderView& view) override {
            // Base image: SceneColor copied into this pass's target. Always runs,
            // since downstream passes consume this output even with no particles.
            Shared<Shader> copyShader = ShaderLibrary::Get("FullscreenCopy");
            copyShader->Bind();
            copyShader->SetInt("u_Source", 0);
            m_RenderGraph->BindInput("SceneColor", 0);
            Renderer::DrawFullscreenQuad();

            if (view.ParticleBatches.empty()) return;
            if (!m_QuadVAO) InitQuad();

            // World-space screen-aligned billboard axes from the inverse
            // view-projection (same derivation as Renderer::DrawCircle).
            glm::mat4 invViewProj = glm::inverse(ctx.ViewProjection);
            auto unproject = [&](float x, float y) {
                glm::vec4 p = invViewProj * glm::vec4(x, y, 0.0f, 1.0f);
                return glm::vec3(p) / p.w;
            };
            glm::vec3 origin = unproject(0.0f, 0.0f);
            glm::vec3 right = glm::normalize(unproject(1.0f, 0.0f) - origin);
            glm::vec3 up = glm::normalize(unproject(0.0f, 1.0f) - origin);

            Shared<Shader> shader = ShaderLibrary::Get("Particle");
            shader->Bind();
            shader->SetInt("u_SceneDepth", 0);
            shader->SetInt("u_Texture", 1);
            shader->SetVec3("u_CameraRight", right);
            shader->SetVec3("u_CameraUp", up);
            shader->SetVec2("u_ViewportSize", glm::vec2(view.ViewportSize));
            m_RenderGraph->BindInput("GBuffer.Depth", 0);

            RenderCommand::SetDepthTest(false);

            for (const ParticleBatch& batch : view.ParticleBatches)
            {
                if (batch.Instances.empty()) continue;

                if (batch.Additive) RenderCommand::SetAdditiveBlend(true);
                else                RenderCommand::SetAlphaBlend(true);
                shader->SetBool("u_Additive", batch.Additive);

                bool useTexture = batch.Texture && batch.Texture->GetID() != 0;
                shader->SetBool("u_UseTexture", useTexture);
                if (useTexture)
                    RenderCommand::BindTexture2D(1, batch.Texture->GetID());

                Allocation alloc = ctx.InstancePool->Allocate(
                    batch.Instances.data(), batch.Instances.size() * sizeof(ParticleInstance));
                ctx.InstancePool->BindAsSSBO(0, alloc.Offset, alloc.Size);

                m_QuadVAO->Bind();
                RenderCommand::DrawIndexedInstances(m_QuadVAO, (unsigned int)batch.Instances.size());
            }

            RenderCommand::SetAdditiveBlend(false); // disables GL_BLEND
            RenderCommand::SetDepthTest(true);
        }

    private:
        void InitQuad() {
            float vertices[] = {
                // corner        uv
                -0.5f, -0.5f,    0.0f, 0.0f,
                 0.5f, -0.5f,    1.0f, 0.0f,
                 0.5f,  0.5f,    1.0f, 1.0f,
                -0.5f,  0.5f,    0.0f, 1.0f,
            };
            unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

            m_QuadVAO = VertexArray::Create();
            Shared<VertexBuffer> vbo = VertexBuffer::Create(vertices, sizeof(vertices));
            vbo->SetLayout({
                { ShaderDataType::Float2, "a_Corner"   },
                { ShaderDataType::Float2, "a_TexCoord" }
            });
            m_QuadVAO->AddVertexBuffer(vbo);
            m_QuadVAO->SetIndexBuffer(IndexBuffer::Create(indices, 6));
        }

        Shared<VertexArray> m_QuadVAO;
    };

}
