#pragma once

#include "pch.h"
#include "KeyCodes.h"
#include "MouseButtonCodes.h"

namespace Engine
{

    class Input
    {
        public:
        static void Update();

        static bool IsKeyPressed(unsigned int keycode);
        static bool IsKeyDown(unsigned int keycode);
        static bool IsKeyReleased(unsigned int keycode);

        static bool IsMouseButtonPressed(unsigned int button);
        static bool IsMouseButtonDown(unsigned int button);
        static bool IsMouseButtonReleased(unsigned int button);

        static std::pair<float, float> GetMousePosition();
        static std::pair<float, float> GetMouseDelta();
        static float GetMouseX();
        static float GetMouseY();

        // Locks and hides the cursor
        static void SetCursorLocked(bool locked);
    };

};
