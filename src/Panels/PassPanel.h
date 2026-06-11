#pragma once

#include "pch.h"

namespace Engine {

    class SceneRenderer;

    class PassPanel {
    public:
        PassPanel() = default;

        void SetRenderer(Shared<SceneRenderer> renderer);
        unsigned int GetSelectedAttachment();

        void OnImGuiRender();

    private:
        Shared<SceneRenderer> m_SceneRenderer;

        // Selection is stored by resource name and resolved to a texture id every
        // frame; ids are stable across resizes, but the producer can be culled by
        // a recompile.
        std::string m_SelectedResource = "FinalColor";
    };
}
