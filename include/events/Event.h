#pragma once

#include "../../pch.h"

enum class EventType
{
    None = 0,
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
    AppTick, AppUpdate, AppRender,
    KeyPressed, KeyReleased,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

enum EventCategory
{
    None = 0,
    EventCategoryApplication = 2,
    EventCategoryInput = 4,
    EventCategoryKeyboard = 8,
    EventCategoryMouse = 16,
    EventCategoryMouseButton = 32

};

#define EVENT_CLASS_TYPE(type) static EventType getStaticType() { return EventType::type; }\
				virtual EventType getEventType() const override { return getStaticType(); }\
				virtual const char* getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int getCategoryFlags() const override { return category; }

class Event
{
    friend class EventDispatcher;

    public:
    virtual EventType getEventType() const = 0;
    virtual const char* getName() const = 0;
    virtual int getCategoryFlags() const = 0;
    virtual std::string toString() const { return getName(); }

    bool isInCategory(EventCategory category)
    {
        return getCategoryFlags() & category;
    };

    protected:
    bool m_Handled = false;
};

class EventDispatcher
{
    template<typename T>
    using EventFunction = std::function<bool(T&)>;
    public:
    EventDispatcher(Event &e) : m_Event(e) {}

    template<typename T>
    bool dispatch(EventFunction<T> fn)
    {
        if (m_Event.getEventType() == T::getStaticType())
        {
            m_Event.m_Handled = fn(*(T*)&m_Event);
            return true;
        }
        return false;
    }

    private:
    Event &m_Event;
};

