#pragma once

#include "LinearMath/btVector3.h"
#include "RigidBody.h"
#include <vector>
#include <memory>
#include <bullet/src/btBulletDynamicsCommon.h>
#include <entt/src/entt/entt.hpp>

namespace Engine {

    struct CollisionData
    {
        void* OtherPointer;
        glm::vec3 HitPoint;
        glm::vec3 Normal;
        float Impulse;
    };

    class Scene;

    using Callback = std::function<void(void*, void*, glm::vec3&, glm::vec3&)>;

    class PhysicsWorld {
    public:
        PhysicsWorld(const btVector3& gravity = btVector3(0, -9.81f, 0));
        PhysicsWorld(float x, float y, float z);
        ~PhysicsWorld();

        void stepSimulation(float deltaTime, int maxSubSteps = 10, float fixedTimeStep = 1.0f / 60.0f);

        void AddRigidBody(RigidBody* body);
        void RemoveRigidBody(RigidBody* body);

        void RegisterCollisionCallback(Callback cb) { m_Callback = cb; }
        void CheckCollisions();

        btDiscreteDynamicsWorld* getWorld() { return world.get(); }

        Scene* GetScene() { return m_Scene; }
        void SetScene(Scene* scene) { m_Scene = scene; }

        bool IsActive() { return m_Active; }
        void SetActive(bool active) { m_Active = active; }

    private:
        // Bullet core objects
        std::unique_ptr<btDefaultCollisionConfiguration> collisionConfig;
        std::unique_ptr<btCollisionDispatcher> dispatcher;
        std::unique_ptr<btDbvtBroadphase> broadphase;
        std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
        std::unique_ptr<btDiscreteDynamicsWorld> world;

        // Keep ownership of created shapes and motion states so they live as long as the rigid bodies
        std::vector<std::unique_ptr<btCollisionShape>> collisionShapes;
        std::vector<std::unique_ptr<btCompoundShape>> compoundShapes;
        std::vector<std::unique_ptr<btDefaultMotionState>> motionStates;

        Callback m_Callback;
        Scene* m_Scene;

        bool m_Active = false;
    };

}
