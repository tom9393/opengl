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

    static void SetOperation(ImGuizmo::OPERATION operation)
    {
        s_Operation = operation;
    }

    static GizmoResult Manipulate(const EditorCamera& camera, glm::mat4& entityTransform, bool snap) {
        GizmoResult result;


        float snapValue = (s_Operation == ImGuizmo::OPERATION::ROTATE) ? 45.0f : 0.5f;
        float snapValues[3] = { snapValue, snapValue, snapValue };

        ImGuizmo::Manipulate(
            glm::value_ptr(camera.GetViewMatrix()),
            glm::value_ptr(camera.GetProjection()),
            s_Operation,
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

    private:
    inline static ImGuizmo::OPERATION s_Operation = ImGuizmo::OPERATION::TRANSLATE;
};

}