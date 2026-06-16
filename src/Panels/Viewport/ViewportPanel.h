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
        enum class Tab { Layout, Modeling, UV };

        void SetActiveTab(Tab tab);

        PassPanel m_PassPanel;
        ObjectViewportPanel m_Object;
        ModelingViewportPanel m_Modeling;
        UVViewportPanel m_UV;

        ViewportPanelBase* m_Tabs[3];

        Tab m_ActiveTab = Tab::Layout;
    };

}
