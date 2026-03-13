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
            void OnCollision(CollisionData &data) override
            {
                TagComponent &tagComponent = GetComponent<TagComponent>();

                entt::entity otherEnt = (entt::entity)(uintptr_t)data.OtherPointer;
                Entity other = { otherEnt, m_Scene };

                RigidBodyComponent &rbc = other.GetComponent<RigidBodyComponent>();

                if (rbc.runtimeBody)
                {
                    RigidBody* rb = (RigidBody*)rbc.runtimeBody;

                    glm::vec3 trans;
                    glm::vec3 rot;

                    rb->GetTransform(trans, rot);
                    trans.y -= 1;
                    rb->SetTransform(trans, rot);
                }
            }
    };
}
