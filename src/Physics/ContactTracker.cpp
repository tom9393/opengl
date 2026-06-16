#include "ContactTracker.h"
#include "PhysicsEngine.h"
#include "RigidBody.h"
#include <bullet/src/btBulletDynamicsCommon.h>

namespace Engine {

    void ContactTracker::Update(btCollisionDispatcher* dispatcher)
    {
        // Pairs touching this frame, so we can diff against m_ActivePairs
        std::unordered_map<uint64_t, ContactEvent> current;

        int numManifolds = dispatcher->getNumManifolds();
        for (int i = 0; i < numManifolds; i++) {
            // Manifold -> which two objects are touching, may have 1 (sphere) or 4 (cube) points of touch
            // Point -> simple geometrical point in space where two manifolds are intersecting

            btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);

            // Make sure they are actually touching
            if (contactManifold->getNumContacts() == 0) continue;

            const btCollisionObject* objA = contactManifold->getBody0();
            const btCollisionObject* objB = contactManifold->getBody1();

            void* ptrA = objA->getUserPointer();
            void* ptrB = objB->getUserPointer();
            if (!ptrA || !ptrB) continue;

            // Get the first contact point for now
            btManifoldPoint& cp = contactManifold->getContactPoint(0);

            entt::entity a = UserPointerToEntity(ptrA);
            entt::entity b = UserPointerToEntity(ptrB);
            uint32_t ua = (uint32_t)a, ub = (uint32_t)b;
            uint64_t key = ua < ub ? ((uint64_t)ua << 32 | ub) : ((uint64_t)ub << 32 | ua);

            // A body flagged with CF_NO_CONTACT_RESPONSE (see RigidBody::SetTrigger) makes this a trigger overlap
            bool trigger = (objA->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE) || (objB->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE);

            current[key] = { a, b, BulletToGlm(cp.m_positionWorldOnB), BulletToGlm(cp.m_normalWorldOnB), cp.m_appliedImpulse, true, trigger };
        }

        if (m_Callback)
        {
            for (auto& [key, event] : current)
                if (m_ActivePairs.find(key) == m_ActivePairs.end())
                    m_Callback(event);

            // Exit events reuse the contact data from the last frame the pair touched.
            for (auto& [key, event] : m_ActivePairs)
                if (current.find(key) == current.end())
                {
                    ContactEvent exit = event;
                    exit.Entered = false;
                    m_Callback(exit);
                }
        }

        m_ActivePairs = std::move(current);
    }

    void ContactTracker::Clear()
    {
        m_Callback = nullptr;
        m_ActivePairs.clear();
    }

}
