#pragma once

#include "pch.h"

namespace Engine {
    class SceneRenderer;

    class StatsPanel {
    public:
        StatsPanel() = default;
        void SetRenderer(Shared<SceneRenderer> renderer);
        void OnImGuiRender();

    private:
        Shared<SceneRenderer> m_SceneRenderer;
    };
}
