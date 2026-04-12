#pragma once
#include <functional>

namespace Engine {
    class ToolbarPanel {
    public:
        ToolbarPanel() = default;

        void SetPlayCallback(const std::function<void()>& callback);
        void SetStopCallback(const std::function<void()>& callback);

        void OnImGuiRender(bool isPlaying);

    private:
        std::function<void()> m_OnPlayCallback;
        std::function<void()> m_OnStopCallback;
    };
}