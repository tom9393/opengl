#pragma once

#include "pch.h"
#include "Asset/AssetManager.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "DrawVec3.h"

namespace Engine {

    inline static void DrawLight(Entity &entity, LightComponent &component)
    {
        const char* lightTypeStrings[] = { "Point", "Directional", "Cone" };
        const char* currentLightTypeString = lightTypeStrings[(int)component.Type];

        if (ImGui::BeginCombo("Type", currentLightTypeString))
        {
            for (int i = 0; i < 3; i++)
            {
                bool isSelected = currentLightTypeString == lightTypeStrings[i];
                if (ImGui::Selectable(lightTypeStrings[i], isSelected))
                {
                    currentLightTypeString = lightTypeStrings[i];
                    component.Type = (LightComponent::LightType)i;
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // Color
        glm::vec3 color = component.Color;
        DrawVec3("Color", color, 0.0f);
        component.Color = color;

        // Intensity
        ImGui::DragFloat("Intensity", &component.Intensity, 0.1f, 0.0f, 10000.0f);

        // Conditionally draw Cone properties
        if (component.Type == LightComponent::LightType::Cone)
        {
            // Note: These are stored as cosines, so the values generally range from 0.0 to 1.0
            ImGui::DragFloat("Inner CutOff", &component.InnerCutOff, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Outer CutOff", &component.OuterCutOff, 0.01f, 0.0f, 1.0f);
        }
    }
}