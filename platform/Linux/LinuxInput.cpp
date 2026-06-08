#include <GLFW/glfw3.h>

#include "Core/Application.h"
#include "Core/Input.h"

namespace Engine
{

    namespace
    {
        constexpr int kMaxKey = KEY_LAST + 1;
        constexpr int kMaxMouse = MOUSE_BUTTON_LAST + 1;

        bool s_CurrKeys[kMaxKey] = {};
        bool s_PrevKeys[kMaxKey] = {};
        bool s_CurrMouse[kMaxMouse] = {};
        bool s_PrevMouse[kMaxMouse] = {};

        float s_CurrMouseX = 0.0f, s_CurrMouseY = 0.0f;
        float s_PrevMouseX = 0.0f, s_PrevMouseY = 0.0f;
        bool s_FirstUpdate = true;
    }

    void Input::Update()
    {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

        // GLFW_KEY_SPACE is the lowest valid key; lower codes would raise GLFW_INVALID_ENUM.
        for (int k = KEY_SPACE; k < kMaxKey; k++)
        {
            s_PrevKeys[k] = s_CurrKeys[k];
            s_CurrKeys[k] = glfwGetKey(window, k) == GLFW_PRESS;
        }

        for (int b = 0; b < kMaxMouse; b++)
        {
            s_PrevMouse[b] = s_CurrMouse[b];
            s_CurrMouse[b] = glfwGetMouseButton(window, b) == GLFW_PRESS;
        }

        double x, y;
        glfwGetCursorPos(window, &x, &y);
        s_PrevMouseX = s_CurrMouseX;
        s_PrevMouseY = s_CurrMouseY;
        s_CurrMouseX = (float)x;
        s_CurrMouseY = (float)y;

        // Avoid a large bogus delta on the very first frame.
        if (s_FirstUpdate)
        {
            s_PrevMouseX = s_CurrMouseX;
            s_PrevMouseY = s_CurrMouseY;
            s_FirstUpdate = false;
        }
    };

    bool Input::IsKeyPressed(unsigned int keycode)
    {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetKey(window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    };

    bool Input::IsKeyDown(unsigned int keycode)
    {
        if (keycode >= kMaxKey) return false;
        return s_CurrKeys[keycode] && !s_PrevKeys[keycode];
    };

    bool Input::IsKeyReleased(unsigned int keycode)
    {
        if (keycode >= kMaxKey) return false;
        return !s_CurrKeys[keycode] && s_PrevKeys[keycode];
    };

    bool Input::IsMouseButtonPressed(unsigned int button)
    {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    };

    bool Input::IsMouseButtonDown(unsigned int button)
    {
        if (button >= kMaxMouse) return false;
        return s_CurrMouse[button] && !s_PrevMouse[button];
    };

    bool Input::IsMouseButtonReleased(unsigned int button)
    {
        if (button >= kMaxMouse) return false;
        return !s_CurrMouse[button] && s_PrevMouse[button];
    };

    std::pair<float, float> Input::GetMouseDelta()
    {
        return { s_CurrMouseX - s_PrevMouseX, s_CurrMouseY - s_PrevMouseY };
    };

    void Input::SetCursorLocked(bool locked)
    {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        glfwSetInputMode(window, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    };

    float Input::GetMouseX()
    {
        auto [x, y] = GetMousePosition();
        return x;
    };

    float Input::GetMouseY()
    {
        auto [x, y] = GetMousePosition();
        return y;
    };

    std::pair<float, float> Input::GetMousePosition()
    {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return { (float)xpos, (float)ypos };
    };

};
