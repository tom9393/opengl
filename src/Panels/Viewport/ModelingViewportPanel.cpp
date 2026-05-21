#include "ModelingViewportPanel.h"
#include "PassPanel.h"

namespace Engine {

    unsigned int ModelingViewportPanel::GetDisplayedTextureID() {
        return m_PassPanel ? m_PassPanel->GetSelectedAttachment() : 0;
    }

}
