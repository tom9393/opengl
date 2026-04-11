#include "Directory.h"

namespace Engine {

	static const std::filesystem::path s_AssetPath = "../assets";

    Directory::Directory() : m_CurrentDirectory(s_AssetPath)
    {
		m_DirIcon = AssetManager::GetAsset<Texture2D>(AssetManager::ImportAsset("../assets/icons/folderIcon.png"));
		m_FileIcon = AssetManager::GetAsset<Texture2D>(AssetManager::ImportAsset("../assets/icons/documentIcon.png"));
		std::static_pointer_cast<OpenGLTexture2D>(m_DirIcon)->Initialize();
		std::static_pointer_cast<OpenGLTexture2D>(m_FileIcon)->Initialize();
    }

    void Directory::OnImGuiRender()
    {
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

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (entry.is_directory())
				{
					// Navigate into the directory
					m_CurrentDirectory /= path.filename();
				}
				else
				{
					// If it's a file, pass it to the AssetManager to import
					Engine::AssetManager::ImportAsset(path);
                    
                    // Optional: You could log something here to confirm the import
                    std::cout << "Imported: " << path.filename() << std::endl;
				}
			}
			ImGui::TextWrapped(path.filename().c_str());
			ImGui::NextColumn();
			ImGui::PopID();
		};

		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);
    }
}