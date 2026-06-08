#include "ContentBrowserPanel.h"
#include "AssetPreviewManager.h"
#include "EditorIcons.h"
#include "Render/Renderer.h"

namespace Engine
{

	ContentBrowserPanel::ContentBrowserPanel()
	{
		EditorIcons::Init();
		AssetPreviewManager::Init();
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
