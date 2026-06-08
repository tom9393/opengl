#include "Directory.h"
#include "EditorIcons.h"

namespace Engine {

	static const std::filesystem::path s_AssetPath = "../assets";

    Directory::Directory() : m_CurrentDirectory(s_AssetPath)
    {
		RefreshCache();
    }

    void Directory::OnImGuiRender()
    {
		PROFILE_FUNCTION();
		if (m_CurrentDirectory != std::filesystem::path(s_AssetPath))
		{
			if (ImGui::Button("Back"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
				RefreshCache();
			};
		};

		float imageSize = 128.0f;
		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		for (auto& item : m_EditorCache) {
			Shared<Texture2D> icon = item.IsDirectory ? EditorIcons::Folder() : EditorIcons::File();

			ImGui::PushID(item.Name.c_str());
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton("Image", (ImTextureID)icon->GetID(), { imageSize, imageSize }, { 0, 1 }, { 1, 0 });
			ImGui::PopStyleColor();

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
				if (item.IsDirectory) {
					m_CurrentDirectory /= item.Path.filename();
					RefreshCache();
				}
				else
				{
					AssetManager::ImportAsset(item.Path);
                    std::cout << "Imported: " << item.Path << std::endl;
				}
			}
			ImGui::TextWrapped(item.Name.c_str());
			ImGui::NextColumn();
			ImGui::PopID();
		}

		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);
    }

	void Directory::RefreshCache()
	{
		m_EditorCache.clear();
		for (auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
			m_EditorCache.push_back({
				entry.path(),
				entry.path().filename().string(),
				entry.is_directory()
			});
		}
	}
}