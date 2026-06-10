#include "InputBindings.h"

#include "pch.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"

namespace Engine {

    void InputBindings::Register(sol::state& state) {
        // Create a Lua table (namespace) for Input
        sol::table input = state.create_table("Input");

        // Bind Input functions
        input.set_function("IsKeyPressed", &Input::IsKeyPressed);
        input.set_function("IsKeyDown", &Input::IsKeyDown);
        input.set_function("IsKeyReleased", &Input::IsKeyReleased);
        input.set_function("IsMouseButtonPressed", &Input::IsMouseButtonPressed);
        input.set_function("IsMouseButtonDown", &Input::IsMouseButtonDown);
        input.set_function("IsMouseButtonReleased", &Input::IsMouseButtonReleased);
        input.set_function("GetMousePosition", &Input::GetMousePosition);
        input.set_function("GetMouseDelta", &Input::GetMouseDelta);
        input.set_function("SetCursorLocked", &Input::SetCursorLocked);

        // Bind Math Utility
        input.set_function("radians", [](float degrees) { return glm::radians(degrees); });

        // Expose KeyCodes as an enum
        state.new_enum("Key",
            "A", KEY_A, "B", KEY_B, "C", KEY_C, "D", KEY_D, "E", KEY_E,
            "F", KEY_F, "G", KEY_G, "H", KEY_H, "I", KEY_I, "J", KEY_J,
            "K", KEY_K, "L", KEY_L, "M", KEY_M, "N", KEY_N, "O", KEY_O,
            "P", KEY_P, "Q", KEY_Q, "R", KEY_R, "S", KEY_S, "T", KEY_T,
            "U", KEY_U, "V", KEY_V, "W", KEY_W, "X", KEY_X, "Y", KEY_Y, "Z", KEY_Z,
            "Key0", KEY_0, "Key1", KEY_1, "Key2", KEY_2, "Key3", KEY_3, "Key4", KEY_4,
            "Key5", KEY_5, "Key6", KEY_6, "Key7", KEY_7, "Key8", KEY_8, "Key9", KEY_9,
            "F1", KEY_F1, "F2", KEY_F2, "F3", KEY_F3, "F4", KEY_F4, "F5", KEY_F5, "F6", KEY_F6,
            "F7", KEY_F7, "F8", KEY_F8, "F9", KEY_F9, "F10", KEY_F10, "F11", KEY_F11, "F12", KEY_F12,
            "Space", KEY_SPACE, "Enter", KEY_ENTER, "Tab", KEY_TAB, "Escape", KEY_ESCAPE,
            "Backspace", KEY_BACKSPACE, "Delete", KEY_DELETE,
            "Left", KEY_LEFT, "Right", KEY_RIGHT, "Up", KEY_UP, "Down", KEY_DOWN,
            "LeftShift", KEY_LEFT_SHIFT, "RightShift", KEY_RIGHT_SHIFT,
            "LeftCtrl", KEY_LEFT_CONTROL, "RightCtrl", KEY_RIGHT_CONTROL,
            "LeftAlt", KEY_LEFT_ALT, "RightAlt", KEY_RIGHT_ALT,
            "Ctrl", KEY_LEFT_CONTROL, "Shift", KEY_LEFT_SHIFT, "Alt", KEY_LEFT_ALT
        );

        // Expose mouse buttons as an enum
        state.new_enum("Mouse",
            "Left", MOUSE_BUTTON_1,
            "Right", MOUSE_BUTTON_2,
            "Middle", MOUSE_BUTTON_3
        );
    }

}
