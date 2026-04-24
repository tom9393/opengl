#pragma once

#include "../../pch.h"
#include "../../Scene/Components.h"
#include "../../Scene/Entity.h"
#include "../../Asset/AssetManager.h"
#include "../../Asset/CollisionBaker.h"

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
            std::vector<Shared<Mesh>> meshes;
            std::vector<Entity> meshEntities;

            if (entity.HasComponent<MeshComponent>()) meshEntities.emplace_back(entity);

            entity.GetAllChildrenWithComponent<MeshComponent>(meshEntities);
            for (auto& e : meshEntities)
            {
                UUID handle = e.GetComponent<MeshComponent>().MeshHandle;
                meshes.emplace_back(AssetManager::GetAsset<Mesh>(handle));
            }

            AssetMetadata metadata = AssetManager::GetMetadata(meshEntities[0].GetComponent<MeshComponent>().MeshHandle);

            // 1. Ensure a bake exists (or bake it now if missing)
            CollisionBaker::Bake(metadata.FilePath, meshes);
        }
        // Add a small hint so the user knows what this does
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Recalculates the simplified physics hull from the high-poly mesh.");

        if (ImGui::DragFloat("Mass", &component.mass))
        {
        }
    }
}