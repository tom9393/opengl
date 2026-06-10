#pragma once

#include "pch.h"
#include "Asset/AssetManager.h"
#include "Asset/Texture.h"
#include "Commands/SceneCommands.h"
#include "DrawVec3.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "Scene/ParticleSystem.h"

namespace Engine {

    inline static void DrawParticleEmitter(Entity &entity, ParticleEmitterComponent &component)
    {
        ImGui::Checkbox("Looping", &component.Looping);
        if (component.Looping)
        {
            ImGui::DragFloat("Rate", &component.Rate, 0.5f, 0.0f, 10000.0f);
        }
        else
        {
            int burst = (int)component.BurstCount;
            if (ImGui::DragInt("Burst Count", &burst, 1, 0, 100000))
                component.BurstCount = (uint32_t)std::max(burst, 0);
        }

        int maxParticles = (int)component.MaxParticles;
        if (ImGui::DragInt("Max Particles", &maxParticles, 10, 1, 1000000))
            component.MaxParticles = (uint32_t)std::max(maxParticles, 1);

        ImGui::DragFloat("Lifetime", &component.Lifetime, 0.05f, 0.01f, 120.0f);

        glm::vec3 direction = component.Direction;
        DrawVec3("Direction", direction, 0.0f);
        component.Direction = direction;

        ImGui::DragFloat("Spread", &component.Spread, 0.5f, 0.0f, 90.0f);
        ImGui::DragFloat("Speed", &component.Speed, 0.05f, 0.0f, 1000.0f);

        glm::vec3 gravity = component.Gravity;
        DrawVec3("Gravity", gravity, 0.0f);
        component.Gravity = gravity;

        ImGui::DragFloat("Start Size", &component.StartSize, 0.01f, 0.0f, 100.0f);
        ImGui::DragFloat("End Size", &component.EndSize, 0.01f, 0.0f, 100.0f);
        ImGui::ColorEdit4("Start Color", glm::value_ptr(component.StartColor));
        ImGui::ColorEdit4("End Color", glm::value_ptr(component.EndColor));
        ImGui::Checkbox("Additive", &component.Additive);

        // Texture slot (optional; soft circle when unset)
        ImGui::Text("Texture");
        ImGui::SameLine();
        Shared<Texture2D> tex = component.TextureHandle
            ? AssetManager::GetAsset<Texture2D>(component.TextureHandle) : nullptr;
        if (tex && tex->GetID())
            ImGui::ImageButton("##particletex", (ImTextureID)(uintptr_t)tex->GetID(), ImVec2(64, 64), {0, 1}, {1, 0});
        else
            ImGui::Button("Drop\nTexture", ImVec2(64, 64));

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetType::Texture2D"))
            {
                UUID texHandle = *(UUID*)payload->Data;
                RecordEntityEdit(entity, [&]{ component.TextureHandle = texHandle; });
            }
            ImGui::EndDragDropTarget();
        }
        if (component.TextureHandle)
        {
            ImGui::SameLine();
            if (ImGui::Button("X"))
                RecordEntityEdit(entity, [&]{ component.TextureHandle = 0; });
        }

        // Preview controls drive the transient runtime state; not undoable.
        if (component.Runtime)
        {
            ImGui::Separator();
            if (ImGui::Button("Burst"))
                component.Runtime->Emit(component.BurstCount);
            ImGui::SameLine();
            if (component.Runtime->IsPlaying())
            {
                if (ImGui::Button("Stop")) component.Runtime->Stop();
            }
            else
            {
                if (ImGui::Button("Play")) component.Runtime->Play();
            }
        }
    }
}
