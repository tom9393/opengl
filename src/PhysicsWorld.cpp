#include "PhysicsWorld.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "RigidBody.h"

namespace Engine {

    PhysicsWorld::PhysicsWorld(const btVector3& gravity)
    {
        collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
        dispatcher = std::make_unique<btCollisionDispatcher>(collisionConfig.get());
        broadphase = std::make_unique<btDbvtBroadphase>();
        solver = std::make_unique<btSequentialImpulseConstraintSolver>();
        world = std::make_unique<btDiscreteDynamicsWorld>(dispatcher.get(), broadphase.get(), solver.get(), collisionConfig.get());
        world->setGravity(gravity);
        // Optional: tune other parameters here (solver iterations, etc.)
    }

    PhysicsWorld::PhysicsWorld(float x, float y, float z) : PhysicsWorld(btVector3(x, y, z)) {}

    PhysicsWorld::~PhysicsWorld()
    {
        // Remove rigid bodies from world and delete them (world does not free motion states or collision shapes)
        if (world) {
            int numObjects = world->getNumCollisionObjects();
            for (int i = numObjects - 1; i >= 0; --i) {
                btCollisionObject* obj = world->getCollisionObjectArray()[i];
                btRigidBody* body = btRigidBody::upcast(obj);
                if (body && body->getMotionState()) {
                    // motion states we created are stored in motionStates vector and will be freed automatically
                }
                world->removeCollisionObject(obj);
                delete obj; // Note: if you added motion state and shape ownership elsewhere, be careful. Here we take simple approach.
            }
        }
        // collisionShapes and motionStates are unique_ptrs and will be freed automatically
    }

    void PhysicsWorld::stepSimulation(float deltaTime, int maxSubSteps, float fixedTimeStep)
    {
        if (world) world->stepSimulation(deltaTime, maxSubSteps, fixedTimeStep);
    }

    void PhysicsWorld::CheckCollisions()
    {
        int numManifolds = dispatcher->getNumManifolds();
        for (int i = 0; i < numManifolds; i++) {
            btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);
            const btCollisionObject* objA = contactManifold->getBody0();
            const btCollisionObject* objB = contactManifold->getBody1();

            // Only trigger if there are actual contact points
            if (contactManifold->getNumContacts() > 0) {
                entt::entity a = (entt::entity)(uintptr_t)objA->getUserPointer();
                entt::entity b = (entt::entity)(uintptr_t)objB->getUserPointer();

                // Notify the Scene/Scripts
                m_Callback(a, b);
                m_Callback(b, a);
            }
        }
    }

    void PhysicsWorld::AddRigidBody(RigidBody* body)
    {
        if (!body) return;
        world->addRigidBody(body->GetBody().get());
    }

    void PhysicsWorld::RemoveRigidBody(RigidBody* body)
    {
        if (!body) return;
        world->removeRigidBody(body->GetBody().get());
        // Note: associated motion state and collision shape are kept in vectors; if you want to free them here,
        // you'd need to track mapping from body -> indices and erase from vectors too.
    }

}
