#include "PhysicsWorld.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "RigidBody.h"

namespace Engine {

    PhysicsWorld::PhysicsWorld(const btVector3& gravity) {
        collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
        dispatcher = std::make_unique<btCollisionDispatcher>(collisionConfig.get());
        broadphase = std::make_unique<btDbvtBroadphase>();
        solver = std::make_unique<btSequentialImpulseConstraintSolver>();
        world = std::make_unique<btDiscreteDynamicsWorld>(dispatcher.get(), broadphase.get(), solver.get(), collisionConfig.get());
        world->setGravity(gravity);
        // Optional: tune other parameters here (solver iterations, etc.)
    }

    PhysicsWorld::PhysicsWorld(float x, float y, float z) : PhysicsWorld(btVector3(x, y, z)) {}

    PhysicsWorld::~PhysicsWorld() {
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

    void PhysicsWorld::stepSimulation(float deltaTime, int maxSubSteps, float fixedTimeStep) {
        if (world) world->stepSimulation(deltaTime, maxSubSteps, fixedTimeStep);
    }

    void PhysicsWorld::AddRigidBody(RigidBody* body) {
        if (!body) return;
        world->addRigidBody(body->GetBody());
    }

    void PhysicsWorld::RemoveRigidBody(RigidBody* body) {
        if (!body) return;
        world->removeRigidBody(body->GetBody());
        // Note: associated motion state and collision shape are kept in vectors; if you want to free them here,
        // you'd need to track mapping from body -> indices and erase from vectors too.
    }

}
