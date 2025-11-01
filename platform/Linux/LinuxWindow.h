#pragma once

#include "../../pch.h"
#include "../../include/Context.h"
#include "../../include/Window.h"

namespace Engine
{

    class LinuxWindow : public Window
    {
    public:

        LinuxWindow(const WindowProps &props);
        virtual ~LinuxWindow();
        virtual void OnUpdate() override;

        unsigned int GetWidth() const override { return m_Data.width; }
        unsigned int GetHeight() const override { return m_Data.height; }
        void* GetNativeWindow() override;

        void SetEventCallback(const EventCallbackFn &cb) override { m_Data.cb = cb; }
        virtual void SetVsync(bool enabled) override;
        virtual bool IsVsync() override;

        virtual void ToggleCursor() override;
        virtual bool IsCursor() override;

    private:

        virtual void Init(const WindowProps &props);
        virtual void Shutdown();

        GLFWwindow* m_Window;
        Context* m_Context;

        struct WindowData {
            std::string title;
            unsigned int height, width;
            bool VSync;
            bool cursor;
            EventCallbackFn cb;
        };

        WindowData m_Data;

    };

};
