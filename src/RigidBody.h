#pragma once

#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletCollision/CollisionShapes/btCompoundShape.h"
#include "BulletCollision/CollisionShapes/btConvexHullShape.h"
#include "BulletCollision/CollisionShapes/btStaticPlaneShape.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "LinearMath/btDefaultMotionState.h"
#include "Mesh.h"
#include "src/entt/entity/fwd.hpp"

namespace Engine {

    static glm::vec3 BulletToGlm(btVector3 &vec)
    {
        return glm::vec3(vec.getX(), vec.getY(), vec.getZ());
    }

    static btVector3 GlmToBullet(const glm::vec3 &vec)
    {
        return btVector3(vec.x, vec.y, vec.z);
    }

    class RBShape
    {
        public:
        RBShape(btCollisionShape* shape) : m_Shape(shape) {}

        btCollisionShape* GetShape() { return m_Shape; };

        protected:
        btCollisionShape* m_Shape;
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
            btConvexHullShape* hull = static_cast<btConvexHullShape*>(m_Shape);
            for (size_t i = 0; i < vertices.size(); i += stride) {
                hull->addPoint(btVector3(vertices[i], vertices[i+1], vertices[i+2]));
            }
        }
    };

    class RBComplexShape : public RBShape
    {
        public:
        RBComplexShape(const std::vector<Shared<Mesh>> &meshes) : RBShape(new btCompoundShape)
        {
            // Compound shape is made of many different meshes
            btCompoundShape* compoundShape = static_cast<btCompoundShape*>(m_Shape);

            for (const auto& mesh : meshes) {
                uint32_t stride = mesh->GetLayout().GetStride() / sizeof(float);
                RBConvexShape* hull = new RBConvexShape(mesh->GetVertices(), stride);

                btTransform localTrans;
                localTrans.setIdentity();
                compoundShape->addChildShape(localTrans, hull->GetShape());
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

            btDefaultMotionState* motionState = new btDefaultMotionState(btTrans);

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

        private:
        std::unique_ptr<RBShape> m_Shape;
        std::unique_ptr<btRigidBody> m_Body;
    };

}
