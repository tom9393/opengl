#pragma once

#include <sol2/sol.hpp>

#include "pch.h"
#include "Asset/Material.h"
#include "Core/Preprocessor.h"
#include "Core/UUID.h"
#include "Physics/RigidBody.h"
#include "Render/SceneCamera.h"
#include "single_include/entt/entt.hpp"
#include "Scripting/NativeScript.h"
#include "Utils/MathTools.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

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
        glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

        glm::mat4 WorldTransform = glm::mat4(1.0f);

        bool IsDirty = true;

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3 &translation, const glm::vec3 &rotation = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &scale = glm::vec3(1.0f, 1.0f, 1.0f)) : translation(translation), rotation(rotation), scale(scale) {}

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

        void SetPosition(const glm::vec3& position) {
            translation = position;
            IsDirty = true;
        }

        void SetRotation(const glm::vec3& rot) {
            rotation = rot;
            IsDirty = true;
        }

        void SetScale(const glm::vec3& sc) {
            scale = sc;
            IsDirty = true;
        }

        void ApplyMatrix(const glm::mat4 &matrix)
        {
            MathTools::DecomposeMatrix(matrix, translation, rotation, scale);
            IsDirty = true;
        }

        void UpdateWorldTransform(const glm::mat4 &parent)
        {
            WorldTransform = parent * GetTransform();
            IsDirty = false;
        }
    };

    struct RelationshipComponent
    {
        entt::entity Parent = entt::null;
        std::vector<entt::entity> Children;

        RelationshipComponent() = default;
        RelationshipComponent(const RelationshipComponent&) = default;
    };

    struct MeshComponent
    {
        UUID MeshHandle = 0;
        
        MeshComponent() = default;
        MeshComponent(const MeshComponent&) = default;
        MeshComponent(UUID handle) : MeshHandle(handle) {}
    };

    struct TextureComponent
    {
        UUID TextureHandle = 0;

        TextureComponent() = default;
        TextureComponent(const TextureComponent&) = default;
        TextureComponent(UUID handle) : TextureHandle(handle) {}
    };

    struct MaterialComponent
    {
        UUID MaterialHandle = 0;

        MaterialComponent() = default;
        MaterialComponent(const MaterialComponent&) = default;
        MaterialComponent(UUID handle) : MaterialHandle(handle) {}
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

        std::filesystem::path ShapePath;

        Shared<RigidBody> RuntimeBody = nullptr;

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
        UUID ScriptHandle;
        sol::environment Environment; // Holds the specific state for THIS entity

        LuaScriptComponent() = default;
        LuaScriptComponent(const LuaScriptComponent&) = default;
        LuaScriptComponent(UUID handle) : ScriptHandle(handle) {}
    };

    struct LightComponent
    {
        enum class LightType { Point = 0, Directional = 1, Cone = 2 };

        LightType Type = LightType::Point;
        
        glm::vec3 Color = glm::vec3(1.0f);
        float Intensity = 1000.0f; // High for point/cone, usually lower (~1.0f) for directional

        bool CastShadows = true;

        // Only used when Type == Cone
        float InnerCutOff = glm::cos(glm::radians(12.5f));
        float OuterCutOff = glm::cos(glm::radians(17.5f));

        LightComponent() = default;
        LightComponent(const LightComponent&) = default;
    };
};
