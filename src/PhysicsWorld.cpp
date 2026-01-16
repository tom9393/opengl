#include "../include/PhysicsWorld.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"

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

    btRigidBody* PhysicsWorld::createStaticPlane(const glm::vec3 &planeNormal, float planeConstant, const glm::vec3 &localInertia) {
        // Create plane shape
        btCollisionShape* shape = new btStaticPlaneShape((btVector3&)planeNormal, planeConstant);
        collisionShapes.emplace_back(shape);

        btTransform startTransform;
        startTransform.setIdentity();

        // mass 0 => static
        btScalar mass = 0.0f;

        btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
        motionStates.emplace_back(motionState);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, (btVector3&)localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        // Optional properties
        body->setFriction(1.0f);
        body->setRestitution(0.0f);

        world->addRigidBody(body);
        return body;
    }

    btRigidBody* PhysicsWorld::createBoxRigidBody(float mass, const glm::vec3 &halfExtents, const glm::vec3 &transform, const glm::vec3 &rotation) {
        btCollisionShape* shape = new btBoxShape((btVector3&)halfExtents);
        collisionShapes.emplace_back(shape);

        btVector3 localInertia(0,0,0);
        if (mass != 0.0f) {
            shape->calculateLocalInertia(mass, localInertia);
        }
        btTransform btTrans;
        btTrans.setIdentity();
        btTrans.setOrigin({transform.x, transform.y, transform.z});

        btQuaternion btRot;
        btRot.setEulerZYX(rotation.z, rotation.y, rotation.x);
        btTrans.setRotation(btRot);

        btDefaultMotionState* motionState = new btDefaultMotionState(btTrans);
        motionStates.emplace_back(motionState);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        body->setFriction(0.8f);
        body->setRestitution(0.1f);
        body->setActivationState(ACTIVE_TAG);

        world->addRigidBody(body);
        return body;
    }

    btRigidBody* PhysicsWorld::createConvexHullRigidBody(float mass, const std::vector<float>& vertices, uint32_t stride, const glm::vec3 &transform, const glm::vec3 &rotation) {
        auto* shape = new btConvexHullShape();
        
        // stride is the number of floats per vertex (e.g., 8 if layout is Pos/Norm/Tex)
        for (size_t i = 0; i < vertices.size(); i += stride) {
            shape->addPoint(btVector3(vertices[i], vertices[i+1], vertices[i+2]));
        }
        
        collisionShapes.emplace_back(shape);

        btVector3 localInertia(0, 0, 0);
        if (mass != 0.0f) shape->calculateLocalInertia(mass, localInertia);
        btTransform btTrans;
        btTrans.setIdentity();
        btTrans.setOrigin({transform.x, transform.y, transform.z});

        btQuaternion btRot;
        btRot.setEulerZYX(rotation.z, rotation.y, rotation.x);
        btTrans.setRotation(btRot);

        auto* motionState = new btDefaultMotionState(btTrans);
        motionStates.emplace_back(motionState);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
        auto* body = new btRigidBody(rbInfo);
        world->addRigidBody(body);
        return body;
    }

    // src/PhysicsWorld.cpp
    btRigidBody* PhysicsWorld::createModelRigidBody(float mass, const std::vector<Shared<Mesh>>& meshes, const glm::vec3 &transform, const glm::vec3 &rotation) {
        auto compoundShape = std::make_unique<btCompoundShape>();

        for (const auto& mesh : meshes) {
            // Create a hull for this specific mesh
            auto* hull = new btConvexHullShape();
            const auto& vertices = mesh->GetVertices();
            uint32_t stride = mesh->GetLayout().GetStride() / sizeof(float);

            for (size_t i = 0; i < vertices.size(); i += stride) {
                hull->addPoint(btVector3(vertices[i], vertices[i + 1], vertices[i + 2]));
            }

            // Track the hull for cleanup
            collisionShapes.emplace_back(hull);

            // Add to compound shape with an identity local transform
            // (assuming meshes are already relative to the model origin)
            btTransform localTrans;
            localTrans.setIdentity();
            compoundShape->addChildShape(localTrans, hull);
        }

        // CRITICAL: Calculate inertia for the ENTIRE compound shape, not sub-shapes
        btVector3 localInertia(0, 0, 0);
        if (mass != 0.0f) {
            compoundShape->calculateLocalInertia(mass, localInertia);
        }
        btTransform btTrans;
        btTrans.setIdentity();
        btTrans.setOrigin({transform.x, transform.y, transform.z});

        btQuaternion btRot;
        btRot.setEulerZYX(rotation.z, rotation.y, rotation.x);
        btTrans.setRotation(btRot);

        auto* motionState = new btDefaultMotionState(btTrans);
        motionStates.emplace_back(motionState);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, compoundShape.get(), localInertia);
        auto* body = new btRigidBody(rbInfo);
        
        body->setFriction(0.8f);
        body->setActivationState(ACTIVE_TAG);

        world->addRigidBody(body);
        
        // Store compound shape for cleanup
        compoundShapes.emplace_back(std::move(compoundShape));
        
        return body;
    }

    void PhysicsWorld::GetTransform(btRigidBody* body, glm::vec3 &translation, glm::vec3 &rotation)
    {
        btTransform btTransform;
        body->getMotionState()->getWorldTransform(btTransform);

        btVector3 pos = btTransform.getOrigin();
        translation = { pos.getX(), pos.getY(), pos.getZ() };

        btQuaternion q = btTransform.getRotation();
        rotation = glm::eulerAngles(glm::quat(q.getW(), q.getX(), q.getY(), q.getZ()));
    }

    void PhysicsWorld::SetTransform(btRigidBody* body, glm::vec3 &translation, glm::vec3 &rotation, float mass)
    {
        btTransform btTransform;
        btTransform.setIdentity();
        btTransform.setOrigin({translation.x, translation.y, translation.z});

        btQuaternion btRot;
        btRot.setEulerZYX(rotation.z, rotation.y, rotation.x);
        btTransform.setRotation(btRot);
        body->setWorldTransform(btTransform);
         
        if (body->getMotionState())
        {
            body->getMotionState()->setWorldTransform(btTransform);
        }


        btVector3 localInertia(0, 0, 0);
        if (mass != 0.0f) {
            body->getCollisionShape()->calculateLocalInertia(mass, localInertia);
            body->setMassProps(mass, localInertia);
        }

        body->updateInertiaTensor();

        body->setLinearVelocity(btVector3(0, 0, 0));
        body->setAngularVelocity(btVector3(0, 0, 0));
        
        body->activate(true);
    }

    void PhysicsWorld::removeRigidBody(btRigidBody* body) {
        if (!body) return;
        world->removeRigidBody(body);
        delete body;
        // Note: associated motion state and collision shape are kept in vectors; if you want to free them here,
        // you'd need to track mapping from body -> indices and erase from vectors too.
    }

}
