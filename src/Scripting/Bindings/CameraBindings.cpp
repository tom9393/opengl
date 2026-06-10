#include "CameraBindings.h"

#include "pch.h"
#include "Scene/Components.h"

namespace Engine {

    void CameraBindings::Register(sol::state& state)
    {
        state.new_usertype<SceneCameraComponent>("CameraComponent",
            "Primary", &SceneCameraComponent::primary,
            "FixedAspectRatio", &SceneCameraComponent::fixedAspectRatio,
            // Vertical FOV in radians (e.g. zoom or sprint kick effects).
            "GetFOV", [](SceneCameraComponent& camera) {
                return camera.sceneCamera ? camera.sceneCamera->GetPerspectiveVerticalFOV() : 0.0f;
            },
            "SetFOV", [](SceneCameraComponent& camera, float radians) {
                if (camera.sceneCamera) camera.sceneCamera->SetPerspectiveVerticalFOV(radians);
            }
        );
    }

}
