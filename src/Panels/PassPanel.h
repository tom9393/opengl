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
        uint32_t m_AttachmentRendererID = 0;
    };
}