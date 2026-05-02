#pragma once

#include "pch.h"
#include "Asset/AssetManager.h"
#include "DrawVec3.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"

namespace Engine {

    inline static void DrawTransform(Entity &entity, TransformComponent &component)
    {
        glm::vec3 translation = component.translation;
        glm::vec3 rotation = glm::degrees(component.rotation);
        glm::vec3 scale = component.scale;

        bool changed = false;
        changed |= DrawVec3("Translation", translation, 0.0f);
        changed |= DrawVec3("Rotation", rotation, 0.0f);
        changed |= DrawVec3("Scale", scale, 1.0f);

        if (changed)
        {
            component.SetPosition(translation);
            component.SetRotation(glm::radians(rotation));
            component.SetScale(scale);
        }
    }

}