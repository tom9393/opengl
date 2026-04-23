#pragma once

#include "../pch.h"

namespace Engine {
    class ToolbarPanel {
    public:
        ToolbarPanel() = default;

        void SetPlayCallback(const std::function<void()>& callback);
        void SetStopCallback(const std::function<void()>& callback);

        void OnImGuiRender();

    private:
        std::function<void()> m_OnPlayCallback;
        std::function<void()> m_OnStopCallback;
    };
}