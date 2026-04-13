#include "../../pch.h"
#include "ToolbarPanel.h"
#include <imgui.h>
#include "../Utils/SceneManager.h"

namespace Engine {
    void ToolbarPanel::SetPlayCallback(const std::function<void()>& callback) {
        m_OnPlayCallback = callback;
    }

    void ToolbarPanel::SetStopCallback(const std::function<void()>& callback) {
        m_OnStopCallback = callback;
    }

    void ToolbarPanel::OnImGuiRender() {
        ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        bool isPlaying = SceneManager::GetSceneState() == SceneState::Play;

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