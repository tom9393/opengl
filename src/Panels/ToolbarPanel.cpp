#include "../../pch.h"
#include "ToolbarPanel.h"
#include <imgui.h>

namespace Engine {
    void ToolbarPanel::SetPlayCallback(const std::function<void()>& callback) {
        m_OnPlayCallback = callback;
    }

    void ToolbarPanel::SetStopCallback(const std::function<void()>& callback) {
        m_OnStopCallback = callback;
    }

    void ToolbarPanel::OnImGuiRender(bool isPlaying) {
        ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        const char* txt = isPlaying ? "Stop" : "Play";
        if (ImGui::Button(txt)) {
            if (!isPlaying && m_OnPlayCallback) {
                m_OnPlayCallback();
            } else if (isPlaying && m_OnStopCallback) {
                m_OnStopCallback();
            }
        }

        ImGui::End();
    }
}