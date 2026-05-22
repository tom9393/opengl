#pragma once

#include "ViewportPanelBase.h"
#include "Commands/EntitySnapshot.h"
#include "Scene/Entity.h"
#include "Utils/SceneManager.h"

namespace Engine {

    class ObjectViewportPanel : public ViewportPanelBase {
    public:
        ObjectViewportPanel() = default;

        entt::entity RaycastSelection();

    protected:
        void OnDraw() override;
        bool OnMousePressed(MouseButtonPressedEvent& e) override;

    private:
        void UpdateSelectedEntityGizmo();

        // Coalesce a gizmo drag into one undo step: snapshot the transform when the drag
        // begins and record a ModifyEntityCommand when it ends.
        bool m_GizmoUsing = false;
        UUID m_GizmoEntity = 0;
        EntitySnapshot m_GizmoBefore;
    };

}
