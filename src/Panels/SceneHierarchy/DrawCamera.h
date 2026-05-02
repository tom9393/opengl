#pragma once

#include "pch.h"
#include "Asset/AssetManager.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"

namespace Engine {

    inline static void DrawCamera(Entity &entity, SceneCameraComponent &component)
    {
        auto &camera = component.sceneCamera;

        const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
        const char* currentProjectionTypeString = projectionTypeStrings[(int)camera->GetProjectionType()];

        if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
        {
            for (unsigned int i = 0; i < 2; i++)
            {
                bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
                if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
                {
                    currentProjectionTypeString = projectionTypeStrings[i];
                    camera->SetProjectionType((SceneCamera::ProjectionType)i);
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        bool enabled = component.primary;
        if (ImGui::Checkbox("Use", &enabled))
        {
            component.primary = !component.primary;
        }

        if (camera->GetProjectionType() == SceneCamera::ProjectionType::Perspective)
        {
            float fov = glm::degrees(camera->GetPerspectiveVerticalFOV());
            if(ImGui::DragFloat("Fov", &fov))
                camera->SetPerspectiveVerticalFOV(glm::radians(fov));

            float near = camera->GetOrthographicNearClip();
            if(ImGui::DragFloat("Near", &near))
                camera->SetOrthographicNearClip(near);

            float far = camera->GetOrthographicFarClip();
            if(ImGui::DragFloat("Far", &far))
                camera->SetOrthographicFarClip(far);
        }

        if (camera->GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
        {
            float orthoSize = camera->GetOrthographicSize();
            if(ImGui::DragFloat("Size", &orthoSize))
                camera->SetOrthographicSize(orthoSize);

            float near = camera->GetOrthographicNearClip();
            if(ImGui::DragFloat("Near", &near))
                camera->SetOrthographicNearClip(near);

            float far = camera->GetOrthographicFarClip();
            if(ImGui::DragFloat("Far", &far))
                camera->SetOrthographicFarClip(far);
        }
    }
}