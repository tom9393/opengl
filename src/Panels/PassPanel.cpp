#include "PassPanel.h"
#include "Scene/SceneRenderer.h"

namespace Engine {

    void PassPanel::SetRenderer(Shared<SceneRenderer> renderer) {
        m_SceneRenderer = renderer;
    }

    unsigned int PassPanel::GetSelectedAttachment()
    {
        if (!m_SceneRenderer) return 0;

        Shared<FramebufferHandle> fb = m_SceneRenderer->GetRenderGraph().GetPassFramebuffer(m_SelectedPass);
        if (!fb) return 0;

        return m_SelectedIsDepth ? fb->GetDepthAttachmentRendererID()
                                 : fb->GetColorAttachmentRendererID(m_SelectedAttachment);
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
                        m_SelectedPass = framebuffer.first;
                        m_SelectedAttachment = depth;
                        m_SelectedIsDepth = true;
                    }
                    depth++;
                }
                else
                {
                    std::string label = "Color " + framebuffer.first + " " + std::to_string(color);
                    if (ImGui::Button(label.c_str()))
                    {
                        m_SelectedPass = framebuffer.first;
                        m_SelectedAttachment = color;
                        m_SelectedIsDepth = false;
                    }
                    color++;
                }
            }
        }

        ImGui::End();
    }
}
