#include "ContentBrowserPanel.h"

static const std::filesystem::path s_AssetPath = "assets";

ContentBrowserPanel::ContentBrowserPanel() : m_CurrentDirectory(s_AssetPath)
{
	m_DirIcon = Texture2D::Create("/home/tom/Coding/c++/opengl/assets/folderIcon.png", TextureType::None);
	m_FileIcon = Texture2D::Create("/home/tom/Coding/c++/opengl/assets/documentIcon.png", TextureType::None);
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

	for (auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory))
	{
		const auto& path = entry.path();
		auto relativePath = std::filesystem::relative(entry.path(), s_AssetPath);
		Shared<Texture2D> icon = entry.is_directory() ? m_DirIcon : m_FileIcon;

		ImGui::PushID(path.filename().string().c_str());
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::ImageButton("Image", (ImTextureID)icon->GetID(), { imageSize, imageSize }, { 0, 1 }, { 1, 0 });
		ImGui::PopStyleColor();
		if (entry.is_directory())
		{
			if (ImGui::Button(relativePath.filename().string().c_str()))
			{
				m_CurrentDirectory /= path.filename();
			};
		}
		else {
			if (ImGui::Button(relativePath.filename().string().c_str())) {};
		}
		ImGui::PopID();
	};

	ImGui::End();
};
