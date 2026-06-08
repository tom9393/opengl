#include "MathBindings.h"

#include "pch.h"

namespace Engine {

    void MathBindings::Register(sol::state& state) {
        // Bind glm::vec2 with constructors, math operators and helper methods
        state.new_usertype<glm::vec2>("vec2",
            sol::constructors<glm::vec2(float, float)>(),
            "x", &glm::vec2::x,
            "y", &glm::vec2::y,
            sol::meta_function::addition, [](const glm::vec2& a, const glm::vec2& b) { return a + b; },
            sol::meta_function::subtraction, [](const glm::vec2& a, const glm::vec2& b) { return a - b; },
            sol::meta_function::multiplication, sol::overload(
                [](const glm::vec2& a, float b) { return a * b; },
                [](float a, const glm::vec2& b) { return a * b; }
            ),
            sol::meta_function::division, [](const glm::vec2& a, float b) { return a / b; },
            sol::meta_function::unary_minus, [](const glm::vec2& v) { return -v; },
            "length", [](const glm::vec2& v) { return glm::length(v); },
            "normalized", [](const glm::vec2& v) { return glm::normalize(v); },
            "dot", [](const glm::vec2& a, const glm::vec2& b) { return glm::dot(a, b); },
            "distance", [](const glm::vec2& a, const glm::vec2& b) { return glm::distance(a, b); },
            "lerp", [](const glm::vec2& a, const glm::vec2& b, float t) { return glm::mix(a, b, t); }
        );

        // Bind glm::vec3 with constructors, math operators and helper methods
        state.new_usertype<glm::vec3>("vec3",
            sol::constructors<glm::vec3(float, float, float)>(),
            "x", &glm::vec3::x,
            "y", &glm::vec3::y,
            "z", &glm::vec3::z,
            // Meta-functions allow Lua to use standard math operators (+, -, *, /)
            sol::meta_function::addition, [](const glm::vec3& a, const glm::vec3& b) { return a + b; },
            sol::meta_function::subtraction, [](const glm::vec3& a, const glm::vec3& b) { return a - b; },

            // Overload to allow scalar multiplication
            sol::meta_function::multiplication, sol::overload(
            [](const glm::vec3& a, float b) { return a * b; },
            [](float a, const glm::vec3& b) { return a * b; }
            ),

            sol::meta_function::division, [](const glm::vec3& a, float b) { return a / b; },
            sol::meta_function::unary_minus, [](const glm::vec3& v) { return -v; },

            "length", [](const glm::vec3& v) { return glm::length(v); },
            "normalized", [](const glm::vec3& v) { return glm::normalize(v); },
            "dot", [](const glm::vec3& a, const glm::vec3& b) { return glm::dot(a, b); },
            "cross", [](const glm::vec3& a, const glm::vec3& b) { return glm::cross(a, b); },
            "distance", [](const glm::vec3& a, const glm::vec3& b) { return glm::distance(a, b); },
            "lerp", [](const glm::vec3& a, const glm::vec3& b, float t) { return glm::mix(a, b, t); }
        );

        // Bind glm::vec4 with constructors, math operators and helper methods
        state.new_usertype<glm::vec4>("vec4",
            sol::constructors<glm::vec4(float, float, float, float)>(),
            "x", &glm::vec4::x,
            "y", &glm::vec4::y,
            "z", &glm::vec4::z,
            "w", &glm::vec4::w,
            sol::meta_function::addition, [](const glm::vec4& a, const glm::vec4& b) { return a + b; },
            sol::meta_function::subtraction, [](const glm::vec4& a, const glm::vec4& b) { return a - b; },
            sol::meta_function::multiplication, sol::overload(
                [](const glm::vec4& a, float b) { return a * b; },
                [](float a, const glm::vec4& b) { return a * b; }
            ),
            sol::meta_function::division, [](const glm::vec4& a, float b) { return a / b; },
            sol::meta_function::unary_minus, [](const glm::vec4& v) { return -v; },
            "length", [](const glm::vec4& v) { return glm::length(v); },
            "normalized", [](const glm::vec4& v) { return glm::normalize(v); },
            "dot", [](const glm::vec4& a, const glm::vec4& b) { return glm::dot(a, b); }
        );

        // Bind glm::quat. Construct from (w,x,y,z) or from a vec3 of euler radians.
        // Multiplying two quats composes them; quat * vec3 rotates the vector.
        state.new_usertype<glm::quat>("quat",
            sol::constructors<glm::quat(float, float, float, float), glm::quat(const glm::vec3&)>(),
            "w", &glm::quat::w,
            "x", &glm::quat::x,
            "y", &glm::quat::y,
            "z", &glm::quat::z,
            sol::meta_function::multiplication, sol::overload(
                [](const glm::quat& a, const glm::quat& b) { return a * b; },
                [](const glm::quat& q, const glm::vec3& v) { return q * v; }
            ),
            "normalized", [](const glm::quat& q) { return glm::normalize(q); },
            "inverse", [](const glm::quat& q) { return glm::inverse(q); },
            // Euler radians, ready to assign to a TransformComponent's Rotation.
            "toEuler", [](const glm::quat& q) { return glm::eulerAngles(q); }
        );

        // Free math helpers. Lua's math library already covers scalar sin/cos/random/etc.
        state.set_function("lerp", sol::overload(
            [](float a, float b, float t) { return a + (b - a) * t; },
            [](const glm::vec3& a, const glm::vec3& b, float t) { return glm::mix(a, b, t); }
        ));
        state.set_function("clamp", sol::overload(
            [](float x, float lo, float hi) { return glm::clamp(x, lo, hi); },
            [](const glm::vec3& v, const glm::vec3& lo, const glm::vec3& hi) { return glm::clamp(v, lo, hi); }
        ));
        state.set_function("slerp", [](const glm::quat& a, const glm::quat& b, float t) { return glm::slerp(a, b, t); });
        state.set_function("angleAxis", [](float angleRad, const glm::vec3& axis) { return glm::angleAxis(angleRad, glm::normalize(axis)); });
        state.set_function("lookRotation", sol::overload(
            [](const glm::vec3& dir) { return glm::quatLookAt(glm::normalize(dir), glm::vec3(0.0f, 1.0f, 0.0f)); },
            [](const glm::vec3& dir, const glm::vec3& up) { return glm::quatLookAt(glm::normalize(dir), up); }
        ));
        state.set_function("radians", [](float degrees) { return glm::radians(degrees); });
        state.set_function("degrees", [](float radians) { return glm::degrees(radians); });
    }

}
