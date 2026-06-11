#include "StatsPanel.h"
#include "pch.h"
#include "Core/Application.h"
#include "Render/Renderer.h"
#include "Scene/SceneRenderer.h"

namespace Engine {
    void StatsPanel::SetRenderer(Shared<SceneRenderer> renderer) {
        m_SceneRenderer = renderer;
    }

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

        if (m_SceneRenderer) {
            ImGui::Separator();
            ImGui::Text("GPU pass times");
            float total = 0.0f;
            for (auto& timing : m_SceneRenderer->GetRenderGraph().GetPassTimings()) {
                ImGui::Text("%s: %.3f ms", timing.Name.c_str(), timing.GpuMs);
                total += timing.GpuMs;
            }
            ImGui::Text("Total: %.3f ms", total);
        }

        ImGui::End();
    }
}
