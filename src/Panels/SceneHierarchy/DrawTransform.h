#pragma once

#include "../../pch.h"
#include "DrawVec3.h"
#include "../../Scene/Components.h"
#include "../../Scene/Entity.h"
#include "../../Asset/AssetManager.h"

namespace Engine {

    inline static void DrawTransform(Entity &entity, TransformComponent &component)
    {
        glm::vec3 translation = component.translation;
        glm::vec3 rotation = glm::degrees(component.rotation);
        glm::vec3 scale = component.scale;

        DrawVec3("Translation", translation, 0.0f);
        DrawVec3("Rotation", rotation, 0.0f);
        DrawVec3("Scale", scale, 1.0f);

        component.translation = translation;
        component.rotation = glm::radians(rotation);
        component.scale = scale;

    }

}