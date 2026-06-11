#include "PassPanel.h"
#include "Scene/SceneRenderer.h"

namespace Engine {

    void PassPanel::SetRenderer(Shared<SceneRenderer> renderer) {
        m_SceneRenderer = renderer;
    }

    unsigned int PassPanel::GetSelectedAttachment()
    {
        if (!m_SceneRenderer) return 0;
        return m_SceneRenderer->GetRenderGraph().GetResourceTextureID(m_SelectedResource);
    }

    void PassPanel::OnImGuiRender() {
        ImGui::Begin("PassPanel");

        for (auto& [name, info] : m_SceneRenderer->GetRenderGraph().GetResources())
        {
            if (!info.aliasOf.empty()) continue; // shares its source's texture
            if (!info.target) continue;          // producer culled
            if (info.target->GetSpecification().type != BufferTextureType::Texture2D) continue;

            if (ImGui::Button(name.c_str()))
                m_SelectedResource = name;
        }

        ImGui::End();
    }
}
