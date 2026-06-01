#include "ViewportPanel.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Utils/ScopedImGuiStyle.h"

namespace Engine {

    ViewportPanel::ViewportPanel()
    {
        m_Object.SetPassPanel(&m_PassPanel);
        m_Modeling.SetPassPanel(&m_PassPanel);
        // The UV editor shares the Modeling tab's edit session (one mesh, two views).
        m_UV.SetSession(&m_Modeling.GetSession());
    }

    void ViewportPanel::SetViewportContext(const ViewportContext& ctx) {
        m_PassPanel.SetRenderer(ctx.SceneRendererContext);
        m_Object.SetViewportContext(ctx);
        m_Modeling.SetViewportContext(ctx);
    }

    void ViewportPanel::OnImGuiRender() {
        ImGuiWindowFlags window_flags = 0;
        if (Input::IsKeyPressed(KEY_LEFT_CONTROL))
            window_flags |= ImGuiWindowFlags_NoMove;

        m_PassPanel.OnImGuiRender();

        ScopedImGuiStyle noPadding(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
        ImGui::Begin("Viewport", nullptr, window_flags);

        if (ImGui::BeginTabBar("ViewportTabs")) {
            if (ImGui::BeginTabItem("Layout")) {
                SetActiveTab(Tab::Layout);
                m_Object.OnImGuiRender();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Modeling")) {
                SetActiveTab(Tab::Modeling);
                m_Modeling.OnImGuiRender();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("UV Editor")) {
                SetActiveTab(Tab::UV);
                m_UV.OnImGuiRender();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }

    void ViewportPanel::SetActiveTab(Tab tab) {
        if (tab == m_ActiveTab)
            return;

        if (IsEditTab(tab) && !IsEditTab(m_ActiveTab))
            m_Modeling.EnterEditMode();
        else if (!IsEditTab(tab) && IsEditTab(m_ActiveTab))
            m_Modeling.ExitEditMode();

        m_ActiveTab = tab;
    }

}
