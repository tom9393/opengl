#pragma once

#include "../../pch.h"
#include "../preprocessor.h"
#include "../Texture.h"
#include "../mesh.h"
#include "../model.h"

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

struct TextureComponent
{
    Shared<Texture2D> texture = nullptr;
    TextureComponent() = default;
    TextureComponent(const TextureComponent&) = default;
    TextureComponent(const Shared<Texture2D> &texture) : texture(texture) {}
};
