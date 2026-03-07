#pragma once

#include "../../pch.h"
#include "../Preprocessor.h"
#include "SceneCamera.h"
#include "../MathTools.h"
#include "UUID.h"

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
        glm::vec3 translation;
        glm::vec3 rotation;
        glm::vec3 scale;

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3 &translation = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &rotation = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &scale = glm::vec3(1.0f, 1.0f, 1.0f)) : translation(translation), rotation(rotation), scale(scale) {}

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
};
