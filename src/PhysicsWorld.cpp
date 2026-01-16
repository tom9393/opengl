#include "../include/PhysicsWorld.h"

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

    btRigidBody* PhysicsWorld::createStaticPlane(const btVector3& planeNormal, btScalar planeConstant, const btVector3&) {
        // Create plane shape
        btCollisionShape* shape = new btStaticPlaneShape(planeNormal, planeConstant);
        collisionShapes.emplace_back(shape);

        btTransform startTransform;
        startTransform.setIdentity();

        // mass 0 => static
        btScalar mass = 0.0f;
        btVector3 localInertia(0,0,0);

        btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
        motionStates.emplace_back(motionState);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        // Optional properties
        body->setFriction(1.0f);
        body->setRestitution(0.0f);

        world->addRigidBody(body);
        return body;
    }

    btRigidBody* PhysicsWorld::createBoxRigidBody(btScalar mass, const btVector3& halfExtents, const btTransform& initialTransform) {
        btCollisionShape* shape = new btBoxShape(halfExtents);
        collisionShapes.emplace_back(shape);

        btVector3 localInertia(0,0,0);
        if (mass != 0.0f) {
            shape->calculateLocalInertia(mass, localInertia);
        }

        btDefaultMotionState* motionState = new btDefaultMotionState(initialTransform);
        motionStates.emplace_back(motionState);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        // Tweak physical properties for your tank/airplane feel
        body->setFriction(0.8f);
        body->setRestitution(0.1f);
        body->setActivationState(ACTIVE_TAG);

        world->addRigidBody(body);
        return body;
    }

    btRigidBody* PhysicsWorld::createConvexHullRigidBody(btScalar mass, const std::vector<float>& vertices, uint32_t stride, const btTransform& initialTransform) {
        auto* shape = new btConvexHullShape();
        
        // stride is the number of floats per vertex (e.g., 8 if layout is Pos/Norm/Tex)
        for (size_t i = 0; i < vertices.size(); i += stride) {
            shape->addPoint(btVector3(vertices[i], vertices[i+1], vertices[i+2]));
        }
        
        collisionShapes.emplace_back(shape);

        btVector3 localInertia(0, 0, 0);
        if (mass != 0.0f) shape->calculateLocalInertia(mass, localInertia);

        auto* motionState = new btDefaultMotionState(initialTransform);
        motionStates.emplace_back(motionState);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
        auto* body = new btRigidBody(rbInfo);
        world->addRigidBody(body);
        return body;
    }

    void PhysicsWorld::removeRigidBody(btRigidBody* body) {
        if (!body) return;
        world->removeRigidBody(body);
        delete body;
        // Note: associated motion state and collision shape are kept in vectors; if you want to free them here,
        // you'd need to track mapping from body -> indices and erase from vectors too.
    }

}
