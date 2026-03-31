#pragma once

#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletCollision/CollisionShapes/btCompoundShape.h"
#include "BulletCollision/CollisionShapes/btConvexHullShape.h"
#include "BulletCollision/CollisionShapes/btStaticPlaneShape.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "LinearMath/btDefaultMotionState.h"
#include "../Asset/Mesh.h"
#include "src/entt/entity/fwd.hpp"

namespace Engine {

    static glm::vec3 BulletToGlm(const btVector3 &vec) { return glm::vec3(vec.getX(), vec.getY(), vec.getZ()); }
    static btVector3 GlmToBullet(const glm::vec3 &vec) { return btVector3(vec.x, vec.y, vec.z); }

    // Todo: fix all of this, leaking a lot of memory
    class RBShape
    {
        public:
        RBShape(btCollisionShape* shape) : m_Shape(shape) {}

        btCollisionShape* GetShape() { return m_Shape.get(); };

        protected:
        std::unique_ptr<btCollisionShape> m_Shape;
    };

    class RBPlaneShape : public RBShape
    {
        public:
        RBPlaneShape(const glm::vec3 &normal, float constant) : RBShape(new btStaticPlaneShape(GlmToBullet(normal), constant)) {};
    };

    class RBBoxShape : public RBShape
    {
        public:
        RBBoxShape(const glm::vec3 &extents) : RBShape(new btBoxShape(GlmToBullet(extents))) {};
    };

    class RBConvexShape : public RBShape
    {
        public:
        RBConvexShape(const std::vector<float> &vertices, float stride) : RBShape(new btConvexHullShape) 
        {
            btConvexHullShape* hull = static_cast<btConvexHullShape*>(m_Shape.get());
            for (size_t i = 0; i < vertices.size(); i += stride) {
                hull->addPoint(btVector3(vertices[i], vertices[i+1], vertices[i+2]));
            }
        }
    };

    class RBComplexShape : public RBShape
    {
        public:
        RBComplexShape(const std::string &modelPath) : RBShape(new btCompoundShape)
        {
            std::string bakePath = modelPath + ".pmcol";
            std::ifstream in(bakePath, std::ios::binary);

            if (!in.is_open()) {
                std::cerr << "[Physics] No bake file found for " << modelPath << std::endl;
                return;
            }

            btCompoundShape* compound = static_cast<btCompoundShape*>(m_Shape.get());
            uint32_t meshCount;
            in.read((char*)&meshCount, sizeof(uint32_t));

            for (uint32_t i = 0; i < meshCount; i++) {
                uint32_t vCount;
                in.read((char*)&vCount, sizeof(uint32_t));
                
                btConvexHullShape* hull = new btConvexHullShape();
                for (uint32_t j = 0; j < vCount; j++) {
                    glm::vec3 v;
                    in.read((char*)&v, sizeof(glm::vec3));
                    hull->addPoint(btVector3(v.x, v.y, v.z));
                }

                btTransform localTrans;
                localTrans.setIdentity();
                compound->addChildShape(localTrans, hull);
            }
        }
    };

    class RigidBody
    {
        public:
        RigidBody(RBShape* shape, const glm::vec3 &translation = glm::vec3(0, 0, 0), const glm::vec3 &rotation = glm::vec3(0, 0, 0), const glm::vec3 &inertia = glm::vec3(0, 0, 0), float mass = 0.0f) : m_Shape(shape)
        {
            btVector3 localInertia = GlmToBullet(inertia);
            if (mass > 0.0f) {
                m_Shape->GetShape()->calculateLocalInertia(mass, localInertia);
            }

            btTransform btTrans;
            btTrans.setIdentity();
            btTrans.setOrigin({translation.x, translation.y, translation.z});

            btQuaternion btRot;
            btRot.setEulerZYX(rotation.z, rotation.y, rotation.x);
            btTrans.setRotation(btRot);

            btDefaultMotionState* motionState = new btDefaultMotionState(btTrans); // Have to use normal pointer, unique causes crash

            btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, m_Shape->GetShape(), localInertia);
            m_Body = std::make_unique<btRigidBody>(rbInfo);
            
            m_Body->setFriction(0.8f);
            m_Body->setActivationState(ACTIVE_TAG);
        }

        std::unique_ptr<btRigidBody> &GetBody() { return m_Body; };

        void SetTransform(const glm::vec3 &translation, const glm::vec3 &rotation)
        {
            btTransform btTransform;
            btTransform.setIdentity();
            btTransform.setOrigin({translation.x, translation.y, translation.z});

            btQuaternion btRot;
            btRot.setEulerZYX(rotation.z, rotation.y, rotation.x);
            btTransform.setRotation(btRot);

            m_Body->setWorldTransform(btTransform);
            if (m_Body->getMotionState())
            {
                m_Body->getMotionState()->setWorldTransform(btTransform);
            }
            m_Body->activate(true);
        }

        void GetTransform(glm::vec3 &translation, glm::vec3 &rotation)
        {
            btTransform btTransform;
            m_Body->getMotionState()->getWorldTransform(btTransform);

            btVector3 pos = btTransform.getOrigin();
            translation = { pos.getX(), pos.getY(), pos.getZ() };

            btQuaternion q = btTransform.getRotation();
            rotation = glm::eulerAngles(glm::quat(q.getW(), q.getX(), q.getY(), q.getZ()));
        }

        void SetMass(float mass)
        {
            btVector3 localInertia(0, 0, 0);
            if (mass > 0.0f) {
                m_Body->getCollisionShape()->calculateLocalInertia(mass, localInertia);
                m_Body->setMassProps(mass, localInertia);
            } else {
                m_Body->setMassProps(0.0f, btVector3(0, 0, 0));
            }

            m_Body->updateInertiaTensor();

            m_Body->setLinearVelocity(btVector3(0, 0, 0));
            m_Body->setAngularVelocity(btVector3(0, 0, 0));

            m_Body->activate(true);
        }

        // Applies a continuous force (e.g., wind, a thruster)
        void ApplyForce(const glm::vec3& force, const glm::vec3& relPos) {
            m_Body->activate(true);
            m_Body->applyForce(GlmToBullet(force), GlmToBullet(relPos));
        }

        // Applies an instantaneous change in momentum (e.g., an explosion or a jump)
        void ApplyImpulse(const glm::vec3& impulse, const glm::vec3& relPos) {
            m_Body->activate(true);
            m_Body->applyImpulse(GlmToBullet(impulse), GlmToBullet(relPos));
        }

        // Applies rotational force
        void ApplyTorque(const glm::vec3& torque) {
            m_Body->activate(true);
            m_Body->applyTorque(GlmToBullet(torque));
        }

        // --- Velocity Control ---

        void SetLinearVelocity(const glm::vec3& velocity) {
            m_Body->activate(true);
            m_Body->setLinearVelocity(GlmToBullet(velocity));
        }

        glm::vec3 GetLinearVelocity() const {
            return BulletToGlm(m_Body->getLinearVelocity());
        }

        void SetAngularVelocity(const glm::vec3& velocity) {
            m_Body->activate(true);
            m_Body->setAngularVelocity(GlmToBullet(velocity));
        }

        // --- Physical Properties ---

        // Damping simulates air resistance (0.0 = no resistance, 1.0 = instant stop)
        void SetDamping(float linear, float angular) {
            m_Body->setDamping(linear, angular);
        }

        void SetRestitution(float restitution) {
            m_Body->setRestitution(restitution); // Bounciness
        }

        void SetFriction(float friction) {
            m_Body->setFriction(friction);
        }

        // Locks movement on specific axes (e.g., for a 2D game in a 3D engine)
        void SetLinearFactor(const glm::vec3& factor) {
            m_Body->setLinearFactor(GlmToBullet(factor));
        }

        void SetAngularFactor(const glm::vec3& factor) {
            m_Body->setAngularFactor(GlmToBullet(factor));
        }

        // --- State Management ---

        void ClearForces() {
            m_Body->clearForces();
        }

        void SetGravity(const glm::vec3& gravity) {
            m_Body->setGravity(GlmToBullet(gravity));
        }

        // Kinematic bodies are moved by code, not by physics forces
        void SetKinematic(bool isKinematic) {
            if (isKinematic) {
                m_Body->setCollisionFlags(m_Body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
                m_Body->setActivationState(DISABLE_DEACTIVATION);
            } else {
                m_Body->setCollisionFlags(m_Body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
                m_Body->setActivationState(WANTS_DEACTIVATION);
            }
        }

        private:
        std::unique_ptr<RBShape> m_Shape;
        std::unique_ptr<btRigidBody> m_Body;
    };

}
