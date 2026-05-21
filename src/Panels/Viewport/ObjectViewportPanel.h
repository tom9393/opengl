#pragma once

#include "ViewportPanelBase.h"
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
    };

}
