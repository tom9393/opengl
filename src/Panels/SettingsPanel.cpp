#include "SettingsPanel.h"
#include "../Render/Renderer.h"

namespace Engine {

	void SettingsPanel::OnImGuiRender()
	{
		ImGui::Begin("Settings panel");

		if (ImGui::Button("Enable wireframe"))
		{
			Renderer::ToggleWireframe();
		}

		ImGui::End();
	};

}
