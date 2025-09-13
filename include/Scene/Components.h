#pragma once

#include "../../pch.h"
#include "../preprocessor.h"
#include "../Texture.h"
#include "../mesh.h"
#include "../model.h"
#include "SceneCamera.h"
#include "../EditorCamera.h"
#include "../Buffer.h"

struct TransformComponent
{
    glm::mat4 transform = {1.0f};

    TransformComponent() = default;
    TransformComponent(const TransformComponent&) = default;
    TransformComponent(const glm::mat4 &transform) : transform(transform) {}
};

struct MeshComponent
{
    Shared<Mesh> mesh = nullptr;

    MeshComponent() = default;
    MeshComponent(const MeshComponent&) = default;
    MeshComponent(const Shared<Mesh> &mesh) : mesh(mesh) {}
};

struct ModelComponent
{
    Shared<Model> model = nullptr;

    ModelComponent() = default;
    ModelComponent(const ModelComponent&) = default;
    ModelComponent(const Shared<Model> &model) : model(model) {}
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
