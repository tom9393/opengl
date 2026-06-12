#pragma once

#include "pch.h"
#include <entt/src/entt/entt.hpp>

namespace Engine {

    struct RaycastResult {
        bool Hit;
        entt::entity HitHandle = entt::null;
        glm::vec3 Position;
        glm::vec3 Normal;
    };

    // Stateless spatial queries against the world owned by PhysicsEngine.
    namespace PhysicsQueries {

        // `ignore` skips one entity's body (typically the caster itself).
        RaycastResult Raycast(const glm::vec3 &origin, const glm::vec3 &direction, float distance, entt::entity ignore = entt::null);
        std::vector<RaycastResult> RaycastAll(const glm::vec3 &origin, const glm::vec3 &direction, float distance);
        // Entities whose bodies overlap a sphere (each entity reported once).
        std::vector<entt::entity> OverlapSphere(const glm::vec3 &center, float radius);

    }

}
