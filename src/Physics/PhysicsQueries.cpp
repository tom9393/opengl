#include "PhysicsQueries.h"
#include "BulletCollision/CollisionDispatch/btCollisionObjectWrapper.h"
#include "PhysicsEngine.h"
#include "RigidBody.h"
#include <bullet/src/btBulletDynamicsCommon.h>

namespace Engine {

    // Closest-hit callback that skips one body (e.g. the caster itself).
    struct IgnoreEntityRayCallback : btCollisionWorld::ClosestRayResultCallback
    {
        void* m_Ignore;

        IgnoreEntityRayCallback(const btVector3& from, const btVector3& to, void* ignore)
            : btCollisionWorld::ClosestRayResultCallback(from, to), m_Ignore(ignore) {}

        btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override
        {
            if (rayResult.m_collisionObject->getUserPointer() == m_Ignore)
                return 1.0f;
            return btCollisionWorld::ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
        }
    };

    RaycastResult PhysicsQueries::Raycast(const glm::vec3 &origin, const glm::vec3 &direction, float distance, entt::entity ignore)
    {
        btVector3 rayFrom = GlmToBullet(origin);
        btVector3 rayTo = GlmToBullet(origin + direction * distance);

        // No body carries the encoded null handle as user pointer, so this never matches when ignore is entt::null.
        IgnoreEntityRayCallback callback(rayFrom, rayTo, EntityToUserPointer(ignore));

        PhysicsEngine::GetWorld()->rayTest(rayFrom, rayTo, callback);

        if (callback.hasHit()) {
            // Retrieve the entity handle stored in the Bullet UserPointer
            entt::entity handle = UserPointerToEntity(callback.m_collisionObject->getUserPointer());
            return { true, handle, BulletToGlm(callback.m_hitPointWorld), BulletToGlm(callback.m_hitNormalWorld) };
        }

        return { false, entt::null, glm::vec3(0.0f), glm::vec3(0.0f) };
    }

    std::vector<RaycastResult> PhysicsQueries::RaycastAll(const glm::vec3 &origin, const glm::vec3 &direction, float distance)
    {
        btVector3 rayFrom = GlmToBullet(origin);
        btVector3 rayTo = GlmToBullet(origin + direction * distance);

        btCollisionWorld::AllHitsRayResultCallback callback(rayFrom, rayTo);
        PhysicsEngine::GetWorld()->rayTest(rayFrom, rayTo, callback);

        std::vector<RaycastResult> results;
        for (int i = 0; i < callback.m_collisionObjects.size(); i++)
        {
            entt::entity handle = UserPointerToEntity(callback.m_collisionObjects[i]->getUserPointer());
            results.push_back({ true, handle, BulletToGlm(callback.m_hitPointWorld[i]), BulletToGlm(callback.m_hitNormalWorld[i]) });
        }

        // Bullet reports hits in arbitrary order; sort nearest-first.
        std::sort(results.begin(), results.end(), [&origin](const RaycastResult& a, const RaycastResult& b) {
            return glm::distance(origin, a.Position) < glm::distance(origin, b.Position);
        });

        return results;
    }

    // Collects the entities whose bodies touch the query object.
    struct OverlapResultCallback : btCollisionWorld::ContactResultCallback
    {
        const btCollisionObject* m_Self = nullptr;
        std::vector<entt::entity> Hits;

        btScalar addSingleResult(btManifoldPoint& cp,
            const btCollisionObjectWrapper* colObj0Wrap, int, int,
            const btCollisionObjectWrapper* colObj1Wrap, int, int) override
        {
            const btCollisionObject* other = colObj0Wrap->getCollisionObject() == m_Self
                ? colObj1Wrap->getCollisionObject() : colObj0Wrap->getCollisionObject();

            if (void* ptr = other->getUserPointer())
            {
                entt::entity handle = UserPointerToEntity(ptr);
                if (std::find(Hits.begin(), Hits.end(), handle) == Hits.end())
                    Hits.push_back(handle);
            }
            return 0;
        }
    };

    std::vector<entt::entity> PhysicsQueries::OverlapSphere(const glm::vec3 &center, float radius)
    {
        btSphereShape sphere(radius);
        btCollisionObject query;
        query.setCollisionShape(&sphere);

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(GlmToBullet(center));
        query.setWorldTransform(transform);

        OverlapResultCallback callback;
        callback.m_Self = &query;
        PhysicsEngine::GetWorld()->contactTest(&query, callback);

        return callback.Hits;
    }

}
