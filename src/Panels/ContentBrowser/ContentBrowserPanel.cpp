#include "ContentBrowserPanel.h"
#include "../../Render/Renderer.h"

namespace Engine
{

	ContentBrowserPanel::ContentBrowserPanel()
	{
	};

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content browser panel");

		if (ImGui::Button("Directory View")) m_Tab = Tab::Directory;
		ImGui::SameLine();
		if (ImGui::Button("Loaded Assets")) m_Tab = Tab::LoadedAssets;
		ImGui::Separator();


		if (m_Tab == Tab::Directory)
		{
			m_Directory.OnImGuiRender();
		} else {
			m_LoadedAssets.OnImGuiRender();
		}

		ImGui::End();
	};

};
