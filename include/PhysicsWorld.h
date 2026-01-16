#pragma once

#include "Mesh.h"
#include <vector>
#include <memory>
#include <bullet/src/btBulletDynamicsCommon.h>

namespace Engine {

    // Simple wrapper for a Bullet physics world
    class PhysicsWorld {
    public:
        PhysicsWorld(const btVector3& gravity = btVector3(0, -9.81f, 0));
        ~PhysicsWorld();

        // Step the simulation. typical call from your game loop
        void stepSimulation(float deltaTime, int maxSubSteps = 10, float fixedTimeStep = 1.0f / 60.0f);

        // Create static plane ground: planeNormal and planeConstant (distance along normal)
        // Returns pointer to the created btRigidBody (owned by the world until removed)
        btRigidBody* createStaticPlane(const btVector3& planeNormal, btScalar planeConstant, const btVector3& localInertia = btVector3(0,0,0));

        // Create a box-shaped rigid body. mass == 0 => static, otherwise dynamic
        // halfExtents = size/2 in each axis. initialTransform is the starting transform.
        btRigidBody* createBoxRigidBody(btScalar mass, const btVector3& halfExtents, const btTransform& initialTransform);

        // Create a convex hull from vertex data (for dynamic objects)
        btRigidBody* createConvexHullRigidBody(btScalar mass, const std::vector<float>& vertices, uint32_t stride, const btTransform& initialTransform);

        // Create a triangle mesh (for static objects)
        //btRigidBody* createTriangleMeshRigidBody(const std::vector<float>& vertices, const std::vector<unsigned int>& indices, uint32_t stride, const btTransform& initialTransform);

        btRigidBody* createModelRigidBody(btScalar mass, const std::vector<Shared<Mesh>>& meshes, const btTransform& initialTransform);

        // Remove and delete a rigid body previously added
        void removeRigidBody(btRigidBody* body);

        // Access to the underlying world when you need lower-level calls
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
