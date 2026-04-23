#include "../../pch.h"
#include "StatsPanel.h"
#include "../Render/Renderer.h"
#include "../Core/Application.h"

namespace Engine {
    void StatsPanel::OnImGuiRender() {
        ImGui::Begin("Stats");

        unsigned int DrawCalls = Renderer::GetStatistics().DrawCalls;
        unsigned int MaterialSwitches = Renderer::GetStatistics().MaterialSwitches;
        unsigned int VerticeCount = Renderer::GetStatistics().VerticeCount;
        unsigned int IndicesCount = Renderer::GetStatistics().IndicesCount;
        float delta = Application::Get().GetWindow().GetFrameTime();

        ImGui::Text("FPS count: %f", 1.f / delta);
        ImGui::Text("Drawcalls: %d", DrawCalls);
        ImGui::Text("Material switches: %d", MaterialSwitches);
        ImGui::Text("Vertices count: %d", VerticeCount);
        ImGui::Text("Indices count: %d", IndicesCount);

        ImGui::End();
    }
}