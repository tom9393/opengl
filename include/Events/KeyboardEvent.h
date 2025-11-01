#pragma once

#include "Event.h"

namespace Engine
{

    class KeyboardEvent : public Event
    {
    public:
        int getKeyCode() const { return m_KeyCode; }

        EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
        protected:
        KeyboardEvent(int keycode) : m_KeyCode(keycode) {}
        int m_KeyCode;

    };

    class KeyboardPressEvent : public KeyboardEvent
    {
        public:
        KeyboardPressEvent(int keycode, int repeats) : KeyboardEvent(keycode), m_Repeats(repeats) {}

        int getRepeatCount() const { return m_Repeats; }

        EVENT_CLASS_TYPE(KeyPressed)

        private:
        int m_Repeats;
    };

    class KeyboardReleaseEvent : public KeyboardEvent
    {
        public:
        KeyboardReleaseEvent(int keycode, int repeats) : KeyboardEvent(keycode), m_Repeats(repeats) {}

        int getRepeatCount() const { return m_Repeats; }

        EVENT_CLASS_TYPE(KeyReleased)

        private:
        int m_Repeats;
    };

};
