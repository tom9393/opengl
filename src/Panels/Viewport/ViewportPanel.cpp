#include "ViewportPanel.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Utils/ScopedImGuiStyle.h"

namespace Engine {

    ViewportPanel::ViewportPanel()
    {
        m_Object.SetPassPanel(&m_PassPanel);
        m_Modeling.SetPassPanel(&m_PassPanel);
    }

    void ViewportPanel::SetViewportContext(const ViewportContext& ctx) {
        m_PassPanel.SetRenderer(ctx.SceneRenderer);
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
                m_Object.OnImGuiRender();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Modeling")) {
                m_Modeling.OnImGuiRender();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }

}
