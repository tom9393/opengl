#include "PhysicsWorld.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "RigidBody.h"
#include "Scene/Entity.h"

namespace Engine {

    static glm::vec3 BtToGlmVec3(btVector3 &vec)
    {
        return glm::vec3(vec.getX(), vec.getY(), vec.getZ());
    }

    static btVector3 GlmToBtVec3(glm::vec3 &vec)
    {
        return btVector3(vec.x, vec.y, vec.z);
    }

    PhysicsWorld::PhysicsWorld(const btVector3& gravity)
    {
        collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
        dispatcher = std::make_unique<btCollisionDispatcher>(collisionConfig.get());
        broadphase = std::make_unique<btDbvtBroadphase>();
        solver = std::make_unique<btSequentialImpulseConstraintSolver>();
        world = std::make_unique<btDiscreteDynamicsWorld>(dispatcher.get(), broadphase.get(), solver.get(), collisionConfig.get());
        debug = std::make_unique<PhysicsDebugDrawer>();
        world->setGravity(gravity);
        world->setDebugDrawer(debug.get());
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

    void PhysicsWorld::StepSimulation(float deltaTime, int maxSubSteps, float fixedTimeStep)
    {
        if (!world) return;
        world->stepSimulation(deltaTime, maxSubSteps, fixedTimeStep);
    }

    void PhysicsWorld::RenderDebug(const glm::mat4 &viewProjection)
    {
        if (world && world->getDebugDrawer())
        {
            world->debugDrawWorld();
        }
    }

    void PhysicsWorld::CheckCollisions()
    {
        int numManifolds = dispatcher->getNumManifolds();
        for (int i = 0; i < numManifolds; i++) {
            // Manifold -> which two objects are touching, may have 1 (sphere) or 4 (cube) points of touch
            // Point -> simple geometrical point in space where two manifolds are intersecting

            btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);

            // Make sure they are actually touching
            if (contactManifold->getNumContacts() == 0) continue;

            const btCollisionObject* objA = contactManifold->getBody0();
            const btCollisionObject* objB = contactManifold->getBody1();

            void* ptrA = objA->getUserPointer();
            void* ptrB = objB->getUserPointer();

            // Only trigger if there are actual contact points
            if (!ptrA || !ptrB) continue;

            // Get the first contact point for now
            btManifoldPoint& cp = contactManifold->getContactPoint(0);
            
            // Convert Bullet vectors to your engine's vector type (e.g., glm::vec3)
            glm::vec3 hitPos = BtToGlmVec3(cp.m_normalWorldOnB);
            glm::vec3 normal = BtToGlmVec3(cp.m_normalWorldOnB);
            glm::vec3 flippedNormal = -normal;

            // Notify the Scene/Scripts
            m_Callback(ptrA, ptrB, hitPos, normal);
            m_Callback(ptrB, ptrA, hitPos, flippedNormal);
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