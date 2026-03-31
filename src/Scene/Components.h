#pragma once

#include "../../pch.h"
#include "../Core/Preprocessor.h"
#include "../Asset/Model.h"
#include "../Scripting/NativeScript.h"
#include "../Render/SceneCamera.h"
#include "../Utils/MathTools.h"
#include "../Core/UUID.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <sol2/sol.hpp>

namespace Engine
{
    struct IDComponent
    {
        UUID id;
        
        IDComponent() = default;
        IDComponent(const IDComponent&) = default;
    };

    struct TagComponent
    {
        std::string tag = "";

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string &tag) : tag(tag) {}
    };

    struct TransformComponent
    {
        glm::vec3 translation;
        glm::vec3 rotation;
        glm::vec3 scale;

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3 &translation = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &rotation = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &scale = glm::vec3(1.0f, 1.0f, 1.0f)) : translation(translation), rotation(rotation), scale(scale) {}

        glm::vec3 GetForward() const
        {
            glm::quat orientation = glm::quat(rotation);
            return orientation * glm::vec3(0.0f, 0.0f, -1.0f);
        }

        glm::vec3 GetUp() const { return glm::quat(rotation) * glm::vec3(0.0f, 1.0f, 0.0f); }
        glm::vec3 GetRight() const { return glm::quat(rotation) * glm::vec3(1.0f, 0.0f, 0.0f); }

        glm::mat4 GetTransform() const
        {
            glm::mat4 rot = glm::toMat4(glm::quat(rotation));

            return glm::translate(glm::mat4(1.0f), translation) * rot * glm::scale(glm::mat4(1.0f), scale);
        }

        void ApplyMatrix(const glm::mat4 &matrix)
        {
            MathTools::DecomposeMatrix(matrix, translation, rotation, scale);
        }
    };

    struct ModelComponent
    {
        std::string path = "";
        Shared<Model> model = nullptr;

        ModelComponent() = default;
        ModelComponent(const ModelComponent&) = default;
        ModelComponent(const std::string &path) : path(path) {}
    };

    struct TextureComponent
    {
        std::string path = "";

        TextureComponent() = default;
        TextureComponent(const TextureComponent&) = default;
        TextureComponent(const std::string &path) : path(path) {}
    };

    struct SceneCameraComponent
    {
        Shared<SceneCamera> sceneCamera;
        bool primary = false;
        bool fixedAspectRatio = false;

        SceneCameraComponent() = default;
        SceneCameraComponent(const SceneCameraComponent&) = default;
        SceneCameraComponent(const Shared<SceneCamera> camera) : sceneCamera(camera) {}
    };

    struct RigidBodyComponent
    {
        enum class BodyType { Static = 0, Dynamic, Kinematic };
        BodyType type = BodyType::Static;
        float mass = 0;
        glm::vec3 inertia = glm::vec3(0, 0, 0);
        
        void* runtimeBody = nullptr;

        RigidBodyComponent() = default;
        RigidBodyComponent(const RigidBodyComponent&) = default;
    };

    struct NativeScriptComponent
    {
        NativeScript* instance = nullptr;

        std::function<void(NativeScriptComponent*)> InstantiateScript;
        std::function<void(NativeScriptComponent*)> DestroyScript;

        template<typename T>
        void Bind()
        {
            InstantiateScript = [](NativeScriptComponent* nsc) { 
                nsc->instance = new T(); 
            };
            
            DestroyScript = [](NativeScriptComponent* nsc) { 
                delete (T*)nsc->instance; 
                nsc->instance = nullptr; 
            };
        }
    };

    struct LuaScriptComponent
    {
        std::string FilePath;
        sol::environment Environment; // Holds the specific state for THIS entity

        LuaScriptComponent() = default;
        LuaScriptComponent(const std::string& path) : FilePath(path) {}
    };
};
