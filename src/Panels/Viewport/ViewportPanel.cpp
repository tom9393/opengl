#include "ViewportPanel.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Utils/ScopedImGuiStyle.h"

namespace Engine {

    ViewportPanel::ViewportPanel()
    {
        m_Object.SetPassPanel(&m_PassPanel);
        m_Modeling.SetPassPanel(&m_PassPanel);

        m_Tabs[(int)Tab::Layout]   = &m_Object;
        m_Tabs[(int)Tab::Modeling] = &m_Modeling;
        m_Tabs[(int)Tab::UV]       = &m_UV;
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
            const char* tabNames[] = { "Layout", "Modeling", "UV Editor" };
            for (int i = 0; i < 3; i++) {
                if (ImGui::BeginTabItem(tabNames[i])) {
                    SetActiveTab((Tab)i);
                    m_Tabs[i]->OnImGuiRender();
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }

    void ViewportPanel::SetActiveTab(Tab tab) {
        if (tab == m_ActiveTab)
            return;

        m_Tabs[(int)m_ActiveTab]->Exit();
        m_Tabs[(int)tab]->Enter();

        m_ActiveTab = tab;
    }

}
