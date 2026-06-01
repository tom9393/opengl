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

    private:
        // Modeling and UV Editor share one edit session, so they form an "edit" group:
        // entering either from Layout enters edit mode, returning to Layout commits, and
        // switching between them leaves the session untouched.
        enum class Tab { Layout, Modeling, UV };
        static bool IsEditTab(Tab tab) { return tab == Tab::Modeling || tab == Tab::UV; }

        // Called inside the active tab's block (before its content renders) to drive the
        // edit-mode lifecycle on a tab change.
        void SetActiveTab(Tab tab);

        PassPanel m_PassPanel;
        ObjectViewportPanel m_Object;
        ModelingViewportPanel m_Modeling;
        UVViewportPanel m_UV;

        Tab m_ActiveTab = Tab::Layout;
    };

}
