#include "ScriptEngine.h"
#include "../Input.h"
#include "../Scene/Components.h"

namespace Engine {

    std::unique_ptr<sol::state> ScriptEngine::s_LuaState;

    void ScriptEngine::Init()
    {
        s_LuaState = std::make_unique<sol::state>();
        s_LuaState->open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::package);

        // TODO: Register C++ Engine API to Lua here
        // 1. Bind GLM Vectors
        s_LuaState->new_usertype<glm::vec3>("vec3",
            sol::constructors<glm::vec3(float, float, float)>(),
            "x", &glm::vec3::x,
            "y", &glm::vec3::y,
            "z", &glm::vec3::z,
            sol::meta_function::addition, [](const glm::vec3& a, const glm::vec3& b) { return a + b; },
            sol::meta_function::subtraction, [](const glm::vec3& a, const glm::vec3& b) { return a - b; },
            sol::meta_function::multiplication, [](const glm::vec3& v, float s) { return v * s; }
        );

        // 2. Bind TransformComponent with methods
        s_LuaState->new_usertype<TransformComponent>("TransformComponent",
            "Translation", &TransformComponent::translation,
            "Rotation", &TransformComponent::rotation,
            "Scale", &TransformComponent::scale,
            "GetForward", &TransformComponent::GetForward,
            "GetRight", &TransformComponent::GetRight
        );

        // 3. Bind functions
        s_LuaState->set_function("IsKeyPressed", &Input::IsKeyPressed);
        s_LuaState->set_function("GetMousePosition", &Input::GetMousePosition);

        // 4. Bind Math Utility
        s_LuaState->set_function("radians", [](float degrees) { return glm::radians(degrees); });
    }

    void ScriptEngine::Shutdown()
    {
        s_LuaState = nullptr;
    }

    sol::state& ScriptEngine::GetState() { return *s_LuaState.get(); }

}