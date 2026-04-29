#include "LoadedAssets.h"
#include "AssetPreviewManager.h"
#include "imgui.h"
#include "../../Asset/AssetManager.h"

namespace Engine {

    static ImVec4 GetColorForAssetType(Engine::AssetType type)
    {
        switch (type)
        {
            case Engine::AssetType::Texture2D: return ImVec4(0.8f, 0.2f, 0.2f, 1.0f); // Red
            case Engine::AssetType::Texture3D: return ImVec4(0.9f, 0.4f, 0.4f, 1.0f); // Light Red
            case Engine::AssetType::Scene:     return ImVec4(0.2f, 0.8f, 0.2f, 1.0f); // Green
            case Engine::AssetType::Prefab:     return ImVec4(0.2f, 0.2f, 0.8f, 1.0f); // Blue
            case Engine::AssetType::Mesh:      return ImVec4(0.4f, 0.4f, 0.9f, 1.0f); // Light Blue
            case Engine::AssetType::Material:  return ImVec4(0.8f, 0.8f, 0.2f, 1.0f); // Yellow
            case Engine::AssetType::LuaScript: return ImVec4(0.8f, 0.4f, 0.8f, 1.0f);
            case Engine::AssetType::None:
            default:                           return ImVec4(0.4f, 0.4f, 0.4f, 1.0f); // Grey
        }
    }

    void LoadedAssets::OnImGuiRender()
    {
        static float padding = 16.0f;
        static float thumbnailSize = 128.0f;
        float cellSize = thumbnailSize + padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1)
            columnCount = 1;

        ImGui::Columns(columnCount, 0, false);

        for (const auto& [uuid, asset] : AssetManager::m_AssetRegistry)
        {
            // You can format this string to show the file path if your metadata structure supports it
            std::string displayName = asset.Name;

            if (displayName.length() > 20)
            {
                displayName = displayName.substr(0, 17) + "...";
            }

            ImGui::PushID((int)(uint64_t)uuid);

            ImVec4 bgColor = GetColorForAssetType(asset.Type);

            uint32_t iconID = AssetPreviewManager::GetPreview(asset.Handle);

            ImGui::ImageButton("AssetIcon", 
					(ImTextureID)iconID, 
					{ thumbnailSize, thumbnailSize }, 
					{ 0, 1 }, { 1, 0 }, 
					bgColor // Passing the background color here!
			);

            if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Path: %s", asset.FilePath.c_str());
                //AssetPreviewManager::UpdatePreview(asset.Handle); // Todo
			}

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                UUID payloadUUID = uuid;
                ImGui::SetDragDropPayload(AssetTypeToString(asset.Type).c_str(), &payloadUUID, sizeof(UUID));
                
                ImGui::Text("Dropping %s", displayName.c_str());
                ImGui::EndDragDropSource();
            }

            ImGui::TextWrapped(displayName.c_str());

            if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Path: %s", asset.FilePath.c_str());
			}

			ImGui::NextColumn();
            ImGui::PopID();
        }
        ImGui::Columns(1);

        // Optional: Allow resizing the thumbnails in Loaded Assets view as well
        ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
        ImGui::SliderFloat("Padding", &padding, 0, 32);
    };

}