#pragma once

#include "../Core/Application.h"
#include "../Asset/AssetManager.h"
#include "../Asset/CollisionBaker.h"
#include "../Scene/Components.h"
#include "../Scene/Entity.h"
#include "../Events/EventBroker.h"
#include "../Core/Input.h"
#include "NativeScript.h"
#include "../Core/TimeStep.h"

namespace Engine {

    class PlayerScript : public NativeScript {
        public:
            void OnCreate() override
            {
                std::cout << "Create method of script" << std::endl;
                auto window = (GLFWwindow*)Application::Get().GetWindow().GetNativeWindow();
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                EventBroker::Get().Subscribe<MouseButtonPressedEvent>(BIND_FN(OnClick), this);
            }
            void OnDestroyed() override
            {
                std::cout << "Destroy method of script" << std::endl;
                EventBroker::Get().UnsubscribeAll(this);
            }
            void OnUpdate(TimeStep ts) override
            {
                auto& tc = GetComponent<TransformComponent>();
                auto [mouseX, mouseY] = Input::GetMousePosition();

                if (firstMouse) {
                    lastMouseX = mouseX;
                    lastMouseY = mouseY;
                    firstMouse = false;
                }

                float offsetX = mouseX - lastMouseX;
                float offsetY = mouseY - lastMouseY; // Reverted for Y-axis

                lastMouseX = mouseX;
                lastMouseY = mouseY;

                float sensitivity = 0.0015f;
                offsetX *= sensitivity;
                offsetY *= sensitivity;

                tc.rotation.x -= offsetY; // Mouse Y moves us around X
                tc.rotation.y -= offsetX; // Mouse X moves us around Y

                if (tc.rotation.x > glm::radians(89.0f)) tc.rotation.x = glm::radians(89.0f);
                if (tc.rotation.x < glm::radians(-89.0f)) tc.rotation.x = glm::radians(-89.0f);
                
                float speed = 30.0f; // Units per second
                float distance = speed * ts;

                glm::vec3 forward = tc.GetForward();
                glm::vec3 right   = tc.GetRight();

                // W / S: Forward and Backward
                if (Input::IsKeyPressed(KEY_W))
                    tc.translation += forward * distance;
                if (Input::IsKeyPressed(KEY_S))
                    tc.translation -= forward * distance;

                // A / D: Left and Right (Strafe)
                if (Input::IsKeyPressed(KEY_A))
                    tc.translation -= right * distance;
                if (Input::IsKeyPressed(KEY_D))
                    tc.translation += right * distance;
            }
            void OnCollision(CollisionData &data) override
            {
            }
            void OnClick(MouseButtonEvent &e)
            {
                Entity entity = m_Scene->CreateEntity("New Kawasaki");

                glm::vec3 thistrans = GetComponent<TransformComponent>().translation;

                auto& transformComponent = entity.AddComponent<TransformComponent>(thistrans);
                auto& rigidbodyComponent = entity.AddComponent<RigidBodyComponent>();
                auto& modelComponent = entity.AddComponent<ModelComponent>();

                modelComponent.path = "../assets/kawasaki/kawasaki.obj";
                modelComponent.model = std::static_pointer_cast<Model>(AssetManager::LoadAsset(modelComponent.path));
                rigidbodyComponent.type = RigidBodyComponent::BodyType::Dynamic;
                rigidbodyComponent.mass = 20;

                // 1. Ensure a bake exists (or bake it now if missing)
                std::string bakePath = modelComponent.path + ".pmcol";
                if (!std::filesystem::exists(bakePath)) {
                    CollisionBaker::Bake(modelComponent.path, modelComponent.model->GetMeshes());
                }

                RBComplexShape* shape = new RBComplexShape(modelComponent.path);
                RigidBody* body = new RigidBody(shape, transformComponent.translation, transformComponent.rotation, rigidbodyComponent.inertia, rigidbodyComponent.mass);
                if (!body) return;

                rigidbodyComponent.runtimeBody = body;

                PhysicsEngine::AddRigidBody(body);
                body->GetBody()->setUserPointer((void*)(uintptr_t)entity.GetHandle());
                body->SetTransform(transformComponent.translation, transformComponent.rotation);

                glm::vec3 direction = GetComponent<TransformComponent>().GetForward();
                float force = 100.0f;

                btVector3 impulse = btVector3(direction.x, direction.y, direction.z) * force;
                body->GetBody()->setLinearVelocity(impulse);
                body->GetBody()->activate(true);
            }

        bool firstMouse = true;
        float lastMouseX;
        float lastMouseY;
    
    };
}
