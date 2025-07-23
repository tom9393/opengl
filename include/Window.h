#pragma once

#include "../pch.h"
#include "../include/events/Event.h"

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
    virtual void onUpdate() = 0;

    virtual unsigned int getWidth() const = 0;
    virtual unsigned int getHeight() const = 0;

    virtual void setEventCallback(const EventCallbackFn &cb) = 0;
    virtual void setVsync(bool enabled) = 0;
    virtual bool isVsync() = 0;

    static Window* Create(const WindowProps &props = WindowProps());
};

