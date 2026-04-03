#pragma once

#include "../pch.h"
#include "../Render/EditorCamera.h"

namespace Engine {

    struct GizmoResult {
        bool IsUsing = false;
        bool IsOver = false;
        glm::mat4 Transform;
    };

    class GizmoManager {
    public:
    static void BeginFrame() {
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
    }

    static void SetViewport(const glm::vec2& pos, const glm::vec2& size) {
        ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);
    }

    static GizmoResult Manipulate(const EditorCamera& camera, glm::mat4& entityTransform, int operation, bool snap) {
        GizmoResult result;

        float snapValue = (operation == ImGuizmo::OPERATION::ROTATE) ? 45.0f : 0.5f;
        float snapValues[3] = { snapValue, snapValue, snapValue };

        ImGuizmo::Manipulate(
            glm::value_ptr(camera.GetViewMatrix()),
            glm::value_ptr(camera.GetProjection()),
            (ImGuizmo::OPERATION)operation,
            ImGuizmo::LOCAL,
            glm::value_ptr(entityTransform),
            nullptr,
            snap ? snapValues : nullptr
        );

        result.IsUsing = ImGuizmo::IsUsing();
        result.IsOver = ImGuizmo::IsOver();
        result.Transform = entityTransform;

        return result;
    }
};

}