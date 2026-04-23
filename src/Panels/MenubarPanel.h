#pragma once

#include "../pch.h"

namespace Engine {
    class MenuBarPanel {
    public:
        MenuBarPanel() = default;

        void SetSaveCallback(const std::function<void()>& callback);
        void SetOpenCallback(const std::function<void()>& callback);

        void OnImGuiRender();

    private:
        std::function<void()> m_OnSaveCallback;
        std::function<void()> m_OnOpenCallback;
    };
}