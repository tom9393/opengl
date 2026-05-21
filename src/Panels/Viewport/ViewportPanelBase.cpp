#include "ViewportPanelBase.h"
#include "PassPanel.h"
#include "Events/EventBroker.h"

namespace Engine {

    ViewportPanelBase::ViewportPanelBase()
    {
        EventBroker::Get().Subscribe<MouseButtonPressedEvent>(BIND_FN(OnMouseButtonPressedDispatch), this);
    }

    ViewportPanelBase::~ViewportPanelBase()
    {
        EventBroker::Get().UnsubscribeAll(this);
    }

    void ViewportPanelBase::SetViewportContext(const ViewportContext& ctx) {
        m_Context = ctx.Scene;
        m_SceneRenderer = ctx.SceneRenderer;
        m_EditorCamera = ctx.Camera;
    }

    void ViewportPanelBase::SetPassPanel(PassPanel* passPanel) {
        m_PassPanel = passPanel;
    }

    unsigned int ViewportPanelBase::GetDisplayedTextureID() {
        return m_PassPanel ? m_PassPanel->GetSelectedAttachment() : 0;
    }

    void ViewportPanelBase::OnImGuiRender() {
        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        glm::vec2 newViewportSize = { contentSize.x, contentSize.y };

        if (m_ViewportSize != newViewportSize && newViewportSize.x > 0 && newViewportSize.y > 0) {
            m_ViewportSize = newViewportSize;
            if (m_EditorCamera)
                m_EditorCamera->SetViewportSize((unsigned int)m_ViewportSize.x, (unsigned int)m_ViewportSize.y);
            if (m_SceneRenderer)
                m_SceneRenderer->OnViewportResize((unsigned int)m_ViewportSize.x, (unsigned int)m_ViewportSize.y);
        }

        ImVec2 cursor = ImGui::GetCursorScreenPos();
        m_ViewportBounds[0] = { cursor.x, cursor.y };
        m_ViewportBounds[1] = { cursor.x + m_ViewportSize.x, cursor.y + m_ViewportSize.y };

        unsigned int texID = GetDisplayedTextureID();
        if (texID != 0) {
            ImGui::Image((void*)(intptr_t)texID, ImVec2{m_ViewportSize.x, m_ViewportSize.y}, ImVec2{0, 1}, ImVec2{1, 0});
        } else {
            ImGui::Dummy(ImVec2{m_ViewportSize.x, m_ViewportSize.y});
        }

        m_IsHovered = ImGui::IsItemHovered();
        m_IsFocused = ImGui::IsWindowFocused();
        if (m_EditorCamera)
            m_EditorCamera->SetViewportHovered(m_IsHovered);

        OnDraw();
    }

    bool ViewportPanelBase::OnMouseButtonPressedDispatch(MouseButtonPressedEvent& e) {
        return OnMousePressed(e);
    }

}
