#pragma once

#include "../../pch.h"
#include "../../Scene/Components.h"
#include "../../Scene/Entity.h"
#include "../../Asset/AssetManager.h"

namespace Engine {

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
        if (AssetManager::IsAssetLoaded(handle))
        {
            Shared<Material> material = AssetManager::GetAsset<Material>(handle);
            ImGui::Separator();
            
            // --- 1. Core Material Settings ---
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

            // --- 2. Dynamic Parameters & Textures ---
            if (ImGui::TreeNodeEx("Parameters", ImGuiTreeNodeFlags_DefaultOpen))
            {
                // Track removals outside the loop to avoid iterator invalidation
                bool removeProperty = false;
                PropertyKey propToRemove;
                
                // Draw Scalar Properties
                for (auto& [key, value] : material->m_Properties)
                {
                    ImGui::PushID((int)key);
                    std::string name = PropertyKeyToString(key);

                    // Use ImGui layout to put the remove button on the right
                    float contentWidth = ImGui::GetContentRegionAvail().x;
                    
                    if (std::holds_alternative<float>(value))
                    {
                        ImGui::SetNextItemWidth(contentWidth - 30.0f);
                        ImGui::DragFloat(name.c_str(), &std::get<float>(value), 0.01f);
                    }
                    else if (std::holds_alternative<glm::vec4>(value))
                    {
                        ImGui::SetNextItemWidth(contentWidth - 30.0f);
                        ImGui::ColorEdit4(name.c_str(), glm::value_ptr(std::get<glm::vec4>(value)));
                    }
                    
                    ImGui::SameLine(contentWidth - 20.0f);
                    if (ImGui::Button("-"))
                    {
                        propToRemove = key;
                        removeProperty = true;
                    }
                    ImGui::PopID();
                }
                if (removeProperty) material->m_Properties.erase(propToRemove);

                ImGui::Separator();

                // Draw Texture Maps
                bool removeTexture = false;
                TextureType texToRemove;
                
                for (auto& [type, tex] : material->m_Textures)
                {
                    ImGui::PushID((int)type);
                    std::string name = TextureTypeToString(type);
                    
                    ImGui::Text("%s", name.c_str());
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 90.0f); 
                    
                    void* texID = tex ? (void*)(uintptr_t)tex->GetID() : nullptr;
                    if (tex) 
                    {
                        ImGui::ImageButton(name.c_str(), (ImTextureID)texID, ImVec2(64, 64), {0, 1}, {1, 0});
                    }
                    else 
                    {
                        ImGui::Button("Drop\nTexture", ImVec2(64, 64)); 
                    }

                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetType::Texture2D"))
                        {
                            UUID texHandle = *(UUID*)payload->Data;
                            material->m_Textures[type] = AssetManager::GetAsset<Texture2D>(texHandle);
                        }
                        ImGui::EndDragDropTarget();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("-"))
                    {
                        texToRemove = type;
                        removeTexture = true;
                    }
                    ImGui::PopID();
                }
                if (removeTexture) material->m_Textures.erase(texToRemove);

                ImGui::Separator();

                // --- 3. Adding New Parameters ---
                if (ImGui::Button("Add Parameter/Map", ImVec2(-1, 0)))
                    ImGui::OpenPopup("AddParamPopup");

                if (ImGui::BeginPopup("AddParamPopup"))
                {
                    if (ImGui::BeginMenu("Textures"))
                    {
                        if (ImGui::MenuItem("Albedo Map"))    material->m_Textures[TextureType::Albedo]    = nullptr;
                        if (ImGui::MenuItem("Normal Map"))    material->m_Textures[TextureType::Normal]    = nullptr;
                        if (ImGui::MenuItem("Metallic Map"))  material->m_Textures[TextureType::Metallic]  = nullptr;
                        if (ImGui::MenuItem("Roughness Map")) material->m_Textures[TextureType::Roughness] = nullptr;
                        if (ImGui::MenuItem("AO Map"))        material->m_Textures[TextureType::AO]        = nullptr;
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Scalars"))
                    {
                        if (ImGui::MenuItem("Albedo Tint")) material->m_Properties[PropertyKey::AlbedoTint] = glm::vec4(1.0f);
                        if (ImGui::MenuItem("Emission"))    material->m_Properties[PropertyKey::Emission]   = 0.0f;
                        ImGui::EndMenu();
                    }
                    ImGui::EndPopup();
                }

                ImGui::TreePop();
            }
        }
        else
        {
            ImGui::TextDisabled("No Material Assigned (Drag & Drop a Material Asset)");
        }
    }
    }

}