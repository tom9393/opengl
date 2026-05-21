#pragma once

#include "ObjectViewportPanel.h"
#include "ModelingViewportPanel.h"
#include "PassPanel.h"

namespace Engine {

    class ViewportPanel {
    public:
        ViewportPanel();

        void SetViewportContext(const ViewportContext& ctx);

        void OnImGuiRender();

    private:
        PassPanel m_PassPanel;
        ObjectViewportPanel m_Object;
        ModelingViewportPanel m_Modeling;
    };

}
