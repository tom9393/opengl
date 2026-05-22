#pragma once

#include "ObjectViewportPanel.h"
#include "ModelingViewportPanel.h"
#include "UVViewportPanel.h"
#include "PassPanel.h"

namespace Engine {

    class ViewportPanel {
    public:
        ViewportPanel();

        void SetViewportContext(const ViewportContext& ctx);

        void OnImGuiRender();

        // The mesh-edit session if the Modeling/UV editor is currently active, else
        // null. Lets the editor route undo/redo to edit-mode's local history.
        MeshEditSession* ActiveMeshSession();

    private:
        PassPanel m_PassPanel;
        ObjectViewportPanel m_Object;
        ModelingViewportPanel m_Modeling;
        UVViewportPanel m_UV;
    };

}
