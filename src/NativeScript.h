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

        protected:
        Entity* m_Entity = nullptr;

        friend class Entity;
    };

}
