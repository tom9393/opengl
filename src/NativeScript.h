#pragma once

#include "TimeStep.h"

namespace Engine {

    class Entity;

    class NativeScript
    {
        public:
        virtual void OnCreate() {}
        virtual void OnDestroyed() {}
        virtual void OnUpdate(TimeStep ts) {}

        private:
        Entity* m_Entity = nullptr;
        friend class Entity;
    };

}
