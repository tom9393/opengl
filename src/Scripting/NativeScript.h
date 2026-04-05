#pragma once

#include "../Core/TimeStep.h"
#include "../../lib/entt/single_include/entt/entt.hpp"

namespace Engine {

    class Scene;
    class Entity;
    struct CollisionData;

    class NativeScript
    {
        public:
        virtual void OnCreate() {}
        virtual void OnDestroyed() {}
        virtual void OnUpdate(TimeStep ts) {}
        virtual void OnCollision(CollisionData &data) {}
        
        Scene* GetScene() { return m_Scene; }

        protected:
        entt::entity m_Entity;
        Scene* m_Scene;

        friend class Entity;
        friend class Scene;
        friend class ScriptEngine;
    };

}
