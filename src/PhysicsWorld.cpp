#include "PhysicsWorld.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "RigidBody.h"
#include "Application.h"
#include "Scene/Entity.h"
#include "Utils/ThreadPool.h"

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
        m_CollisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
        m_Dispatcher = std::make_unique<btCollisionDispatcher>(m_CollisionConfig.get());
        m_Broadphase = std::make_unique<btDbvtBroadphase>();
        m_Solver = std::make_unique<btSequentialImpulseConstraintSolver>();
        m_World = std::make_unique<btDiscreteDynamicsWorld>(m_Dispatcher.get(), m_Broadphase.get(), m_Solver.get(), m_CollisionConfig.get());
        m_Debug = std::make_unique<PhysicsDebugDrawer>();
        m_World->setGravity(gravity);
        m_World->setDebugDrawer(m_Debug.get());
    }

    PhysicsWorld::PhysicsWorld(float x, float y, float z) : PhysicsWorld(btVector3(x, y, z)) {}

    PhysicsWorld::~PhysicsWorld()
    {
        // Remove rigid bodies from world and delete them (world does not free motion states or collision shapes)
        if (m_World) {
            int numObjects = m_World->getNumCollisionObjects();
            for (int i = numObjects - 1; i >= 0; --i) {
                btCollisionObject* obj = m_World->getCollisionObjectArray()[i];
                btRigidBody* body = btRigidBody::upcast(obj);
                if (body && body->getMotionState()) {
                    // motion states we created are stored in motionStates vector and will be freed automatically
                }
                m_World->removeCollisionObject(obj);
                delete obj; // Note: if you added motion state and shape ownership elsewhere, be careful. Here we take simple approach.
            }
        }
        // collisionShapes and motionStates are unique_ptrs and will be freed automatically
    }

    void PhysicsWorld::StepSimulation(float deltaTime, int maxSubSteps, float fixedTimeStep)
    {
        if (!m_World) return;
        m_World->stepSimulation(deltaTime, maxSubSteps, fixedTimeStep);
    }

    void PhysicsWorld::RenderDebug(const glm::mat4 &viewProjection)
    {
        if (m_World && m_World->getDebugDrawer())
        {
            m_World->debugDrawWorld();
        }
    }

    void PhysicsWorld::CheckCollisions()
    {
        int numManifolds = m_Dispatcher->getNumManifolds();
        for (int i = 0; i < numManifolds; i++) {
            // Manifold -> which two objects are touching, may have 1 (sphere) or 4 (cube) points of touch
            // Point -> simple geometrical point in space where two manifolds are intersecting

            btPersistentManifold* contactManifold = m_Dispatcher->getManifoldByIndexInternal(i);

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
        m_World->addRigidBody(body->GetBody().get());
    }

    void PhysicsWorld::RemoveRigidBody(RigidBody* body)
    {
        if (!body) return;
        m_World->removeRigidBody(body->GetBody().get());
        // Note: associated motion state and collision shape are kept in vectors; if you want to free them here,
        // you'd need to track mapping from body -> indices and erase from vectors too.
    }

}