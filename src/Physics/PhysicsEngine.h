#pragma once

#include "pch.h"
#include "ContactTracker.h"
#include "Core/TimeStep.h"
#include <entt/src/entt/entt.hpp>

class btDiscreteDynamicsWorld;

namespace Engine {

    // Passed to NativeScript::OnCollision on contact enter
    struct CollisionData
    {
        entt::entity Other = entt::null;
        glm::vec3 HitPoint;
        glm::vec3 Normal;
        float Impulse;
    };

    struct PhysicsWorldConfiguration;

    struct PhysicsSettings
    {
        bool Active;
        bool Debug;
    };

    class Scene;
    class Entity;
    class RigidBody;

    // Entity handles are stored in Bullet user pointers offset by one so the first entt handle (0) is distinguishable from an unset user pointer
    inline void* EntityToUserPointer(entt::entity e) { return (void*)((uintptr_t)e + 1); }
    inline entt::entity UserPointerToEntity(void* ptr) { return (entt::entity)((uintptr_t)ptr - 1); }

    class PhysicsEngine {
    public:

        static void Init();
        static void Shutdown();

        static void OnUpdate(TimeStep ts);

        static void RuntimeStart(Scene* scene);
        static void RuntimeStop();

        static void AddRigidBody(Shared<RigidBody> &body);
        static void RemoveRigidBody(Shared<RigidBody> &body);

        static Shared<RigidBody> AddRuntimeBody(Entity entity);

        static void RegisterContactCallback(ContactCallback cb) { s_Contacts.SetCallback(std::move(cb)); }

        static btDiscreteDynamicsWorld* GetWorld();

        static void RenderDebug(const glm::mat4 &viewProjection);

        static Scene* GetScene() { return s_Scene; }
        static void SetScene(Scene* scene) { s_Scene = scene; }

        static void ToggleDebug() { s_Settings.Debug = !s_Settings.Debug; }

    private:
        static void StepSimulation(float deltaTime, int maxSubSteps = 10, float fixedTimeStep = 1.0f / 60.0f);

    private:

        static std::unique_ptr<PhysicsWorldConfiguration> s_PhysicsWorldConfiguration;
        static Scene* s_Scene;
        static PhysicsSettings s_Settings;

        inline static ContactTracker s_Contacts;
    };

}
