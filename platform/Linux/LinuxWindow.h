#pragma once

#include "../../pch.h"
#include "../../include/Context.h"
#include "../../include/Window.h"

class LinuxWindow : public Window
{
public:

    LinuxWindow(const WindowProps &props);
    virtual ~LinuxWindow();
    virtual void onUpdate() override;

    unsigned int getWidth() const override { return m_Data.width; }
    unsigned int getHeight() const override { return m_Data.height; }
    void* GetNativeWindow() override;

    void setEventCallback(const EventCallbackFn &cb) override { m_Data.cb = cb; }
    virtual void setVsync(bool enabled) override;
    virtual bool isVsync() override;

    virtual void setCursor(bool enabled) override;
    virtual bool isCursor() override;

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

