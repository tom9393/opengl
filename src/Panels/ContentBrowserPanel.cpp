#include "ContentBrowserPanel.h"
#include "AssetManager.h"

namespace Engine
{

	static const std::filesystem::path s_AssetPath = "../assets";

	ContentBrowserPanel::ContentBrowserPanel() : m_CurrentDirectory(s_AssetPath)
	{
		m_DirIcon = std::static_pointer_cast<Texture2D>(AssetManager::LoadAsset("../assets/folderIcon.png"));
		m_FileIcon = std::static_pointer_cast<Texture2D>(AssetManager::LoadAsset("../assets/documentIcon.png"));
	};

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content browser panel");

		if (m_CurrentDirectory != std::filesystem::path(s_AssetPath))
		{
			if (ImGui::Button("Back"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
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

		for (auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = entry.path();
			auto relativePath = std::filesystem::relative(entry.path(), s_AssetPath);
			Shared<Texture2D> icon = entry.is_directory() ? m_DirIcon : m_FileIcon;

			ImGui::PushID(path.filename().string().c_str());
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton("Image", (ImTextureID)icon->GetID(), { imageSize, imageSize }, { 0, 1 }, { 1, 0 });
			ImGui::PopStyleColor();

			if (ImGui::BeginDragDropSource())
			{
				std::filesystem::path relativePath(path);
				const char* itemPath = relativePath.c_str();

				std::filesystem::path itemExtension = relativePath.extension();
				if (itemExtension == ".pmblend")
				{
					ImGui::SetDragDropPayload("SCENE", itemPath, strlen(itemPath) + 1);
				} else if (itemExtension == ".assbin") {
					ImGui::SetDragDropPayload("MODEL", itemPath, strlen(itemPath) + 1);
				} else {
					ImGui::SetDragDropPayload("UNKNOWN_ITEM", itemPath, strlen(itemPath) + 1);
				}

				ImGui::EndDragDropSource();
			}
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (entry.is_directory())
					m_CurrentDirectory /= path.filename();

			}
			ImGui::TextWrapped(path.filename().c_str());
			ImGui::NextColumn();
			ImGui::PopID();
		};

		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);

		ImGui::End();
	};

};
