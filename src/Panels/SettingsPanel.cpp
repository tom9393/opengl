#include "SettingsPanel.h"
#include "Render/Renderer.h"
#include "Physics/PhysicsEngine.h"

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

		ImGui::End();
	};

}
