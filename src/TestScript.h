#pragma once

#include "Components.h"
#include "NativeScript.h"
#include "TimeStep.h"
#include "Entity.h"

namespace Engine {

    class TestScript : public NativeScript {
        public:
            void OnCreate() override
            {
                std::cout << "Create method of script" << std::endl;
            }
            void OnDestroyed() override
            {
                std::cout << "Destroy method of script" << std::endl;
            }
            void OnUpdate(TimeStep ts) override
            {
                TransformComponent& transformComponent = m_Entity->GetComponent<TransformComponent>();
                if (transformComponent.translation.y < 4)
                {
                    std::cout << "The y is less than 2" << std::endl;
                }
            }
            void OnCollision() override
            {
                std::cout << "Test collision function" << std::endl;
            }
    };
}
