#pragma once

#include "pch.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "Asset/AssetManager.h"
#include "Asset/MatdefLoader.h"

namespace Engine {

    static std::string MaterialTypeToString(MaterialType type)
    {
        switch (type)
        {
            case MaterialType::None:                return "None";
            
            case MaterialType::Albedo:              return "Albedo";
            case MaterialType::Normal:              return "Normal";
            case MaterialType::Metallic:            return "Metallic";
            case MaterialType::Roughness:           return "Roughness";
            case MaterialType::AO:                  return "Ambient Occlusion";
            case MaterialType::Height:              return "Height";
            case MaterialType::Emissive:            return "Emissive Map";

            case MaterialType::Diffuse:             return "Diffuse";
            case MaterialType::Specular:            return "Specular";
            case MaterialType::Transparency:        return "Transparency";

            case MaterialType::AlbedoTint:          return "Albedo Tint";
            case MaterialType::Emission:            return "Emission Color";
            case MaterialType::RoughnessMultiplier: return "Roughness Multiplier";
            case MaterialType::MetallicMultiplier:  return "Metallic Multiplier";
            
            default:                                return "Unknown";
        }
    }

	static std::string TextureTypeToString(MaterialType type)
	{
		switch (type)
		{
			case MaterialType::Albedo:       return "Albedo Map";
			case MaterialType::Normal:       return "Normal Map";
			case MaterialType::Metallic:     return "Metallic Map";
			case MaterialType::Roughness:    return "Roughness Map";
			case MaterialType::AO:           return "Ambient Occlusion";
			case MaterialType::Emissive:     return "Emissive Map";
			default:                         return "Texture Map";
		}
	}

    inline static void DrawMaterial(Entity &entity, MaterialComponent &component)
    {
        ImGui::Button("Material", ImVec2(100.0f, 0.0f));
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetType::Material"))
            {
                UUID handle = *(UUID*)payload->Data;
                component.MaterialHandle = handle;
            }
            ImGui::EndDragDropTarget();
        }
        
        UUID handle = component.MaterialHandle;
        if (!AssetManager::IsAssetLoaded(handle))
        {
            Shared<Material> newMaterial = MatdefLoader::LoadMaterial("/home/tom/Coding/c++/opengl/shaders/PBR_Standard.matdef");

            newMaterial->Add({MaterialType::AlbedoTint, glm::vec4(1.0f)});
            newMaterial->Add({MaterialType::Emission, 0.0f});
            newMaterial->Add({MaterialType::RoughnessMultiplier, 0.5f});
            newMaterial->Add({MaterialType::MetallicMultiplier, 0.0f});

            AssetMetadata metadata;
            metadata.Handle = UUID();
            metadata.Name = "Material";
            metadata.Type = AssetType::Material;
            metadata.IsMemoryAsset = true;
            metadata.FilePath = "";

            Shared<Asset> cast = newMaterial;

            AssetManager::RegisterAsset(metadata, cast);

            component.MaterialHandle = metadata.Handle;
            handle = metadata.Handle;
        }

        Shared<Material> material = AssetManager::GetAsset<Material>(handle);
        ImGui::Separator();
        
        if (ImGui::TreeNodeEx("Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const char* blendModes[] = { "Opaque", "Transparent", "Additive" };
            int currentMode = (int)material->mode;
            if (ImGui::Combo("Blend Mode", &currentMode, blendModes, IM_ARRAYSIZE(blendModes)))
                material->mode = (BlendMode)currentMode;

            ImGui::Checkbox("Depth Write", &material->DepthWrite);
            ImGui::Checkbox("Double Sided", &material->DoubleSided);
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Parameters", ImGuiTreeNodeFlags_DefaultOpen))
        {
            MaterialType propToRemove = MaterialType::None;
            bool shouldRemove = false;
            
            float contentWidth = ImGui::GetContentRegionAvail().x;

            for (auto& [key, value] : material->m_Properties)
            {
                ImGui::PushID((int)key);
                std::string name = MaterialTypeToString(key);

                if (std::holds_alternative<float>(value))
                {
                    ImGui::SetNextItemWidth(contentWidth - 30.0f);
                    ImGui::DragFloat(name.c_str(), &std::get<float>(value), 0.01f);
                    ImGui::SameLine(contentWidth - 20.0f);
                }
                else if (std::holds_alternative<glm::vec4>(value))
                {
                    ImGui::SetNextItemWidth(contentWidth - 30.0f);
                    ImGui::ColorEdit4(name.c_str(), glm::value_ptr(std::get<glm::vec4>(value)));
                    ImGui::SameLine(contentWidth - 20.0f);
                }
                else if (std::holds_alternative<Shared<Texture2D>>(value))
                {
                    auto& tex = std::get<Shared<Texture2D>>(value);
                    ImGui::Text("%s", name.c_str());
                    ImGui::SameLine(contentWidth - 90.0f); 
                    
                    void* texID = tex ? (void*)(uintptr_t)tex->GetID() : nullptr;
                    if (tex) 
                        ImGui::ImageButton("##tex", (ImTextureID)texID, ImVec2(64, 64), {0, 1}, {1, 0});
                    else 
                        ImGui::Button("Drop\nTexture", ImVec2(64, 64)); 

                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetType::Texture2D"))
                        {
                            UUID texHandle = *(UUID*)payload->Data;
                            material->Add({key, AssetManager::GetAsset<Texture2D>(texHandle)});
                        }
                        ImGui::EndDragDropTarget();
                    }
                    ImGui::SameLine(contentWidth - 20.0f);
                }

                if (ImGui::Button("-"))
                {
                    propToRemove = key;
                    shouldRemove = true;
                }
                
                ImGui::PopID();
            }

            if (shouldRemove) material->Remove(propToRemove);

            ImGui::Separator();

            if (ImGui::Button("Add Parameter/Map", ImVec2(-1, 0)))
                ImGui::OpenPopup("AddParamPopup");

            if (ImGui::BeginPopup("AddParamPopup"))
            {
                if (ImGui::BeginMenu("Textures"))
                {
                    if (ImGui::MenuItem("Albedo Map"))    material->Add({MaterialType::Albedo, Shared<Texture2D>(nullptr)});
                    if (ImGui::MenuItem("Normal Map"))    material->Add({MaterialType::Normal, Shared<Texture2D>(nullptr)});
                    if (ImGui::MenuItem("Metallic Map"))  material->Add({MaterialType::Metallic, Shared<Texture2D>(nullptr)});
                    if (ImGui::MenuItem("Roughness Map")) material->Add({MaterialType::Roughness, Shared<Texture2D>(nullptr)});
                    if (ImGui::MenuItem("AO Map"))        material->Add({MaterialType::AO, Shared<Texture2D>(nullptr)});
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Properties"))
                {
                    if (ImGui::MenuItem("Albedo Tint"))          material->Add({MaterialType::AlbedoTint, glm::vec4(1.0f)});
                    if (ImGui::MenuItem("Emission Color"))       material->Add({MaterialType::Emission, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)});
                    if (ImGui::MenuItem("Roughness Multiplier")) material->Add({MaterialType::RoughnessMultiplier, 1.0f});
                    if (ImGui::MenuItem("Metallic Multiplier"))  material->Add({MaterialType::MetallicMultiplier, 1.0f});
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }

            ImGui::TreePop();
        }
    };

}