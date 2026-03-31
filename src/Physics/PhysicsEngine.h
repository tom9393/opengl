#pragma once

#include "LinearMath/btVector3.h"
#include "PhysicsDebugDrawer.h"
#include "RigidBody.h"
#include <vector>
#include <memory>
#include <bullet/src/btBulletDynamicsCommon.h>
#include <entt/src/entt/entt.hpp>
#include "../Core/TimeStep.h"

namespace Engine {

    struct CollisionData
    {
        void* OtherPointer;
        glm::vec3 HitPoint;
        glm::vec3 Normal;
        float Impulse;
    };

    struct PhysicsWorldConfiguration
    {
        std::unique_ptr<btDefaultCollisionConfiguration> CollisionConfig;
        std::unique_ptr<btCollisionDispatcher> Dispatcher;
        std::unique_ptr<btDbvtBroadphase> Broadphase;
        std::unique_ptr<btSequentialImpulseConstraintSolver> Solver;
        std::unique_ptr<btDiscreteDynamicsWorld> World;
        std::unique_ptr<PhysicsDebugDrawer> Debug;
    };

    struct PhysicsWorldObjects
    {
        std::vector<std::unique_ptr<btCollisionShape>> CollisionShapes;
        std::vector<std::unique_ptr<btCompoundShape>> CompoundShapes;
        std::vector<std::unique_ptr<btDefaultMotionState>> MotionStates;

    };

    class Scene;

    using Callback = std::function<void(void*, void*, glm::vec3&, glm::vec3&)>;

    class PhysicsEngine {
    public:

        static void Init();
        static void Shutdown();

        static void OnUpdate(TimeStep ts);

        static void RuntimeStart(Scene* scene);
        static void RuntimeStop();

        static void AddRigidBody(RigidBody* body);
        static void RemoveRigidBody(RigidBody* body);

        static void RegisterCollisionCallback(Callback cb) { s_Callback = cb; }
        static void CheckCollisions();

        btDiscreteDynamicsWorld* getWorld() { return s_PhysicsWorldConfiguration->World.get(); }

        static void RenderDebug(const glm::mat4 &viewProjection);

        static Scene* GetScene() { return s_Scene; }
        static void SetScene(Scene* scene) { s_Scene = scene; }

    private:
        static void StepSimulation(float deltaTime, int maxSubSteps = 10, float fixedTimeStep = 1.0f / 60.0f);

    private:

        static std::unique_ptr<PhysicsWorldConfiguration> s_PhysicsWorldConfiguration;
        static std::unique_ptr<PhysicsWorldObjects> s_PhysicsWorldObjects;
        static Scene* s_Scene;

        inline static Callback s_Callback;

        bool m_Active = false;
    };

}
