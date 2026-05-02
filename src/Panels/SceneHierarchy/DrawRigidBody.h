#pragma once

#include "pch.h"
#include "Asset/AssetManager.h"
#include "Asset/CollisionBaker.h"
#include "Core/Application.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"

namespace Engine {

    inline static void DrawRigidBody(Entity &entity, RigidBodyComponent &component)
    {
        const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
        const char* currentBodyTypeString = bodyTypeStrings[(int)component.type];

        if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
        {
            for (unsigned int i = 0; i < 3; i++)
            {
                bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
                if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
                {
                    currentBodyTypeString = bodyTypeStrings[i];
                    component.type = (RigidBodyComponent::BodyType)i;
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        if (ImGui::Button("Re-Bake Collision Data"))
        {
            component.ShapePath = CollisionBaker::Bake(entity);
        }

        bool fileExists = !component.ShapePath.empty() && std::filesystem::exists(component.ShapePath);

        ImGui::SameLine();
        if (fileExists)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
            ImGui::Text("(Meshes baked)");
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::Text("(No bake found)");
            ImGui::PopStyleColor();
        }

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Recalculates the simplified physics hull from the high-poly mesh.");

        if (ImGui::DragFloat("Mass", &component.mass))
        {
        }
    }
}