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
                return;
                // Currently this is just for testing
                RigidBodyComponent& rbc = GetComponent<RigidBodyComponent>();
                
                glm::vec3 transform = glm::vec3(0);
                glm::vec3 rotation = glm::vec3(0);

                auto* body = static_cast<RigidBody*>(rbc.runtimeBody);

                if (body)
                {
                    body->GetTransform(transform, rotation);
                }

                if (transform.y < 2)
                {
                    std::cout << "The y is less than 2" << std::endl;
                    transform.y = 20;
                    body->SetTransform(transform, rotation);
                }
            }
            void OnCollision(Entity &other) override
            {
                TagComponent &tagComponent = GetComponent<TagComponent>();
                std::cout << "I am " << tagComponent.tag << std::endl;
                if (other.HasComponent<TagComponent>()) {
                    auto& tag = other.GetComponent<TagComponent>().tag;
                    std::cout << "I was hit by: " << tag << std::endl;
                }
            }
    };
}
