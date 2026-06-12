#pragma once

#include "pch.h"
#include <entt/src/entt/entt.hpp>

class btCollisionDispatcher;

namespace Engine {

    // One contact pair starting or ending a touch. Normal points from A to B;
    // Impulse is the solver's applied impulse at the first contact point.
    struct ContactEvent
    {
        entt::entity A = entt::null;
        entt::entity B = entt::null;
        glm::vec3 Point;
        glm::vec3 Normal;
        float Impulse = 0.0f;
        bool Entered = true;  // true = enter, false = exit
        bool Trigger = false; // either body is a no-contact-response trigger
    };

    using ContactCallback = std::function<void(const ContactEvent&)>;

    // Diffs the dispatcher's contact manifolds against the previous frame and
    // fires the registered callback with enter/exit events.
    class ContactTracker
    {
    public:
        void SetCallback(ContactCallback cb) { m_Callback = std::move(cb); }
        void Update(btCollisionDispatcher* dispatcher);
        void Clear();

    private:
        ContactCallback m_Callback;

        // Pairs currently touching, keyed by their two entity handles, so we can
        // diff against the previous frame to emit enter/exit events.
        std::unordered_map<uint64_t, ContactEvent> m_ActivePairs;
    };

}
