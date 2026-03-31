#pragma once

#include "../pch.h"
#include "Events/Event.h"

namespace Engine
{

    struct WindowProps
    {
        std::string title;
        unsigned int height;
        unsigned int width;
        WindowProps(const std::string &title = "Default", unsigned int width = 1600, unsigned int height = 1600) : title(title), width(width), height(height) {};
    };

    class Window
    {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        virtual ~Window() {};
        virtual void OnUpdate() = 0;

        virtual unsigned int GetWidth() const = 0;
        virtual unsigned int GetHeight() const = 0;

        virtual void SetEventCallback(const EventCallbackFn &cb) = 0;
        virtual void SetVsync(bool enabled) = 0;
        virtual bool IsVsync() = 0;

        virtual void ToggleCursor() = 0;
        virtual bool IsCursor() = 0;

        virtual void* GetNativeWindow() = 0;

        static Window* Create(const WindowProps &props = WindowProps());
    };

};
