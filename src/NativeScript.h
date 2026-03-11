#pragma once

#include "Scene.h"
#include "TimeStep.h"

namespace Engine {

    class Entity;

    class NativeScript
    {
        public:
        virtual void OnCreate() {}
        virtual void OnDestroyed() {}
        virtual void OnUpdate(TimeStep ts) {}
        virtual void OnCollision(Entity &other) {}
        
        Scene* GetScene() { return m_Scene; }

        template <typename T>
        T& GetComponent() {
            return m_Scene->m_Registry.get<T>(m_Entity);
        }

        private:
        entt::entity m_Entity;
        Scene* m_Scene;

        friend class Entity;
    };

}
