#include "MenubarPanel.h"
#include "Core/Application.h"

namespace Engine {
    void MenuBarPanel::SetSaveCallback(const std::function<void()>& callback) {
        m_OnSaveCallback = callback;
    }

    void MenuBarPanel::SetOpenCallback(const std::function<void()>& callback) {
        m_OnOpenCallback = callback;
    }

    void MenuBarPanel::OnImGuiRender() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Save as..") && m_OnSaveCallback) {
                    m_OnSaveCallback();
                }
                if (ImGui::MenuItem("Open..") && m_OnOpenCallback) {
                    m_OnOpenCallback();
                }
                if (ImGui::MenuItem("Exit")) {
                    Application::Get().Close();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
}