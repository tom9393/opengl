#pragma once

#include "Events/EventBroker.h"
#include "NativeScript.h"
#include "TimeStep.h"

namespace Engine {

    class ShootScript : public NativeScript {
        public:
            void OnCreate() override
            {
                std::cout << "Create method of script" << std::endl;
                EventBroker::Get().Subscribe<MouseButtonPressedEvent>(BIND_FN(OnClick), this);
            }
            void OnDestroyed() override
            {
                std::cout << "Destroy method of script" << std::endl;
                EventBroker::Get().UnsubscribeAll(this);
            }
            void OnUpdate(TimeStep ts) override
            {
            }
            void OnCollision(CollisionData &data) override
            {
            }
            void OnClick(MouseButtonEvent &e)
            {
                Info("Hello from the script");
            }
    };
}
