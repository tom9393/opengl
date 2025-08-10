#pragma once

#include "../pch.h"
#include "../include/KeyCodes.h"
#include "../include/MouseButtonCodes.h"

class Input
{
    public:
    static bool IsKeyPressed(unsigned int keycode);
    static bool IsMouseButtonPressed(unsigned int button);

    static std::pair<float, float> GetMousePosition();
    static float GetMouseX();
    static float GetMouseY();
};
