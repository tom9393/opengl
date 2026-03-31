#pragma once

#include "../Scene/Scene.h"
#include "../Core/TimeStep.h"

namespace Engine {

    class Entity;

    class NativeScript
    {
        public:
        virtual void OnCreate() {}
        virtual void OnDestroyed() {}
        virtual void OnUpdate(TimeStep ts) {}
        virtual void OnCollision(CollisionData &data) {}
        
        Scene* GetScene() { return m_Scene; }

        template <typename T>
        T& GetComponent() {
            return m_Scene->m_Registry.get<T>(m_Entity);
        }

        protected:
        entt::entity m_Entity;
        Scene* m_Scene;

        friend class Entity;
        friend class Scene;
    };

}
