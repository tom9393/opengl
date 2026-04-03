#pragma once

#include "../../pch.h"
#include "../../src/Render/Context.h"
#include "../../src/Core/Window.h"

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

        virtual void UpdateFrameTime() override;
        virtual float GetFrameTime() override;

        virtual void EnableCursor() override;
        virtual void DisableCursor() override;
        virtual bool IsCursor() override;

    private:

        virtual void Init(const WindowProps &props);
        virtual void Shutdown();

        GLFWwindow* m_Window;
        Context* m_Context;

        float m_PreviousFrame = 0;
        float m_CurrentFrame = 0;

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
