#pragma once

#include "RigidBody.h"
#include <vector>
#include <memory>
#include <bullet/src/btBulletDynamicsCommon.h>

namespace Engine {

    class PhysicsWorld {
    public:
        PhysicsWorld(const btVector3& gravity = btVector3(0, -9.81f, 0));
        ~PhysicsWorld();

        void stepSimulation(float deltaTime, int maxSubSteps = 10, float fixedTimeStep = 1.0f / 60.0f);

        void AddRigidBody(RigidBody* body);
        void RemoveRigidBody(RigidBody* body);

        btDiscreteDynamicsWorld* getWorld() { return world.get(); }

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
    };

}
