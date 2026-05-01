#include "PassPanel.h"
#include "Scene/SceneRenderer.h"

namespace Engine {

    void PassPanel::SetRenderer(Shared<SceneRenderer> renderer) {
        m_SceneRenderer = renderer;
    }

    unsigned int PassPanel::GetSelectedAttachment()
    {
        return m_AttachmentRendererID;
    }

    void PassPanel::OnImGuiRender() {
        ImGui::Begin("PassPanel");

        std::unordered_map<std::string, Shared<FramebufferHandle>> framebuffers = m_SceneRenderer->GetRenderGraph().GetFramebuffers();

        for (auto& framebuffer : framebuffers)
        {

            unsigned int color = 0;
            unsigned int depth = 0;

            FramebufferSpecification spec = framebuffer.second->GetFramebuffer()->GetSpecification();

            if (spec.type != BufferTextureType::Texture2D) continue;

            std::vector<FramebufferTextureSpecification> textureSpecs = spec.attachments.Attachments;

            for (auto& specification : textureSpecs)
            {
                if (specification.TextureFormat == FramebufferTextureFormat::DEPTH24STENCIL8 || specification.TextureFormat == FramebufferTextureFormat::Depth)
                {
                    std::string label = "Depth " + framebuffer.first + " " + std::to_string(depth);
                    if (ImGui::Button(label.c_str()))
                    {
                        m_AttachmentRendererID = framebuffer.second->GetDepthAttachmentRendererID();
                    }
                    depth++;
                }
                else
                {
                    std::string label = "Color " + framebuffer.first + " " + std::to_string(color);
                    if (ImGui::Button(label.c_str()))
                    {
                        m_AttachmentRendererID = framebuffer.second->GetColorAttachmentRendererID(color);
                    }
                    color++;
                }
            }
        }

        ImGui::End();
    }
}