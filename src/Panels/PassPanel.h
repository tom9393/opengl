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

        // Selection is stored by pass name + attachment and resolved to a texture id
        // every frame, so it survives framebuffer resizes (which recreate textures).
        std::string m_SelectedPass = "PostProcessPass";
        unsigned int m_SelectedAttachment = 0;
        bool m_SelectedIsDepth = false;
    };
}
