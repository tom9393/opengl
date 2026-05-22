#include "SettingsPanel.h"
#include "Render/Renderer.h"
#include "Physics/PhysicsEngine.h"
#include "Utils/SceneManager.h"
#include "Utils/PlatformUtils.h"

namespace Engine {

	void SettingsPanel::OnImGuiRender()
	{
		ImGui::Begin("Settings panel");

		if (ImGui::Button("Enable wireframe"))
		{
			Renderer::ToggleWireframe();
		}

		if (ImGui::Button("Enable collision debug"))
		{
			PhysicsEngine::ToggleDebug();
		}

		Scene* scene = SceneManager::GetContext();
		if (scene)
		{
			if (ImGui::CollapsingHeader("Environment (IBL)"))
			{
				const std::string& path = scene->GetEnvironmentPath();
				ImGui::TextWrapped("HDR: %s", path.empty() ? "(none)" : path.c_str());

				if (ImGui::Button("Load HDR"))
				{
					std::string file = FileDialogs::OpenFile("*.hdr");
					if (!file.empty()) scene->SetEnvironmentPath(file);
				}
				ImGui::SameLine();
				if (ImGui::Button("Clear"))
					scene->SetEnvironmentPath("");

				ImGui::SliderFloat("Intensity", &scene->EnvironmentIntensity(), 0.0f, 5.0f);
			}

			if (ImGui::CollapsingHeader("Post Processing"))
			{
				PostProcessSettings& post = scene->GetPostSettings();
				ImGui::SliderFloat("Exposure", &post.Exposure, 0.0f, 5.0f);

				ImGui::Checkbox("Bloom", &post.BloomEnabled);
				ImGui::SliderFloat("Bloom Intensity", &post.BloomIntensity, 0.0f, 1.0f);
				ImGui::SliderFloat("Bloom Threshold", &post.BloomThreshold, 0.0f, 5.0f);

				ImGui::SliderFloat("Vignette", &post.VignetteStrength, 0.0f, 1.0f);
				ImGui::SliderFloat("Chromatic Aberration", &post.ChromaticAberration, 0.0f, 5.0f);
			}
		}

		ImGui::End();
	};

}
