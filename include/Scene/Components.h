#pragma once

#include "../../pch.h"
#include "../preprocessor.h"
#include "../Texture.h"
#include "../mesh.h"
#include "../model.h"
#include "SceneCamera.h"
#include "../EditorCamera.h"
#include "../Buffer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

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
};

struct ModelComponent
{
    std::string model = "";

    ModelComponent() = default;
    ModelComponent(const ModelComponent&) = default;
    ModelComponent(const std::string &model) : model(model) {}
};

struct TagComponent
{
    std::string tag = "";

    TagComponent() = default;
    TagComponent(const TagComponent&) = default;
    TagComponent(const std::string &tag) : tag(tag) {}
};

struct TextureComponent
{
    Shared<Texture2D> texture = nullptr;

    TextureComponent() = default;
    TextureComponent(const TextureComponent&) = default;
    TextureComponent(const Shared<Texture2D> &texture) : texture(texture) {}
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

struct EditorCameraComponent
{
    Shared<EditorCamera> editorCamera;
    bool primary = true;
    bool fixedAspectRatio = false;

    EditorCameraComponent() = default;
    EditorCameraComponent(const EditorCameraComponent&) = default;
    EditorCameraComponent(const Shared<EditorCamera> camera) : editorCamera(camera) {}
};
