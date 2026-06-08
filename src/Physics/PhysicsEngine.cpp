#include "PhysicsEngine.h"
#include "Asset/AssetManager.h"
#include "Asset/CollisionBaker.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "Core/Application.h"
#include "PhysicsDebugDrawer.h"
#include "RigidBody.h"
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

    std::unique_ptr<PhysicsWorldConfiguration> PhysicsEngine::s_PhysicsWorldConfiguration;
    std::unique_ptr<PhysicsWorldObjects> PhysicsEngine::s_PhysicsWorldObjects;
    Scene* PhysicsEngine::s_Scene;
    Callback s_Callback;
    PhysicsSettings PhysicsEngine::s_Settings = {true, false};

    void PhysicsEngine::Init()
    {
        s_PhysicsWorldConfiguration = std::make_unique<PhysicsWorldConfiguration>();
        s_PhysicsWorldConfiguration->CollisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
        s_PhysicsWorldConfiguration->Dispatcher = std::make_unique<btCollisionDispatcher>(s_PhysicsWorldConfiguration->CollisionConfig.get());
        s_PhysicsWorldConfiguration->Broadphase = std::make_unique<btDbvtBroadphase>();
        s_PhysicsWorldConfiguration->Solver = std::make_unique<btSequentialImpulseConstraintSolver>();
        s_PhysicsWorldConfiguration->World = std::make_unique<btDiscreteDynamicsWorld>(s_PhysicsWorldConfiguration->Dispatcher.get(), s_PhysicsWorldConfiguration->Broadphase.get(), s_PhysicsWorldConfiguration->Solver.get(), s_PhysicsWorldConfiguration->CollisionConfig.get());
        s_PhysicsWorldConfiguration->Debug = std::make_unique<PhysicsDebugDrawer>();
        s_PhysicsWorldConfiguration->World->setGravity(btVector3(0, -9.81f, 0));
        s_PhysicsWorldConfiguration->World->setDebugDrawer(s_PhysicsWorldConfiguration->Debug.get());
    }

    void PhysicsEngine::Shutdown()
    {
        s_PhysicsWorldConfiguration = nullptr;
        s_PhysicsWorldObjects = nullptr;
        s_Scene = nullptr;
        s_Callback = nullptr;
        s_ContactCallback = nullptr;
        s_ActivePairs.clear();
    }

    void PhysicsEngine::StepSimulation(float deltaTime, int maxSubSteps, float fixedTimeStep)
    {
        if (!s_PhysicsWorldConfiguration->World) return;
        s_PhysicsWorldConfiguration->World->stepSimulation(deltaTime, maxSubSteps, fixedTimeStep);
    }

    void PhysicsEngine::RenderDebug(const glm::mat4 &viewProjection)
    {
        if (s_Settings.Debug && s_PhysicsWorldConfiguration && s_PhysicsWorldConfiguration->World && s_PhysicsWorldConfiguration->World->getDebugDrawer())
        {
            s_PhysicsWorldConfiguration->World->debugDrawWorld();
        }
    }

    void PhysicsEngine::CheckCollisions()
    {
        // Pairs touching this frame, so we can diff against s_ActivePairs for enter/exit.
        std::unordered_map<uint64_t, ContactPair> current;

        int numManifolds = s_PhysicsWorldConfiguration->Dispatcher->getNumManifolds();
        for (int i = 0; i < numManifolds; i++) {
            // Manifold -> which two objects are touching, may have 1 (sphere) or 4 (cube) points of touch
            // Point -> simple geometrical point in space where two manifolds are intersecting

            btPersistentManifold* contactManifold = s_PhysicsWorldConfiguration->Dispatcher->getManifoldByIndexInternal(i);

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
            glm::vec3 normal = BtToGlmVec3(cp.m_normalWorldOnB);
            glm::vec3 flippedNormal = -normal;

            // Per-contact, per-frame notification (used by native scripts' OnCollision)
            if (s_Callback)
            {
                glm::vec3 hitPos = normal;
                s_Callback(ptrA, ptrB, hitPos, normal);
                s_Callback(ptrB, ptrA, hitPos, flippedNormal);
            }

            // Record the pair for enter/exit diffing. A body flagged with
            // CF_NO_CONTACT_RESPONSE (see RigidBody::SetTrigger) makes this a trigger overlap.
            entt::entity a = (entt::entity)(uintptr_t)ptrA;
            entt::entity b = (entt::entity)(uintptr_t)ptrB;
            uint32_t ua = (uint32_t)a, ub = (uint32_t)b;
            uint64_t key = ua < ub ? ((uint64_t)ua << 32 | ub) : ((uint64_t)ub << 32 | ua);

            bool trigger = (objA->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE)
                        || (objB->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE);

            current[key] = { a, b, trigger, BtToGlmVec3(cp.m_positionWorldOnB), normal };
        }

        if (s_ContactCallback)
        {
            for (auto& [key, pair] : current)
                if (s_ActivePairs.find(key) == s_ActivePairs.end())
                    s_ContactCallback(pair.a, pair.b, pair.point, pair.normal, true, pair.trigger);

            for (auto& [key, pair] : s_ActivePairs)
                if (current.find(key) == current.end())
                    s_ContactCallback(pair.a, pair.b, pair.point, pair.normal, false, pair.trigger);
        }

        s_ActivePairs = std::move(current);
    }

    void PhysicsEngine::AddRigidBody(Shared<RigidBody> &body)
    {
        s_PhysicsWorldConfiguration->World->addRigidBody(body->GetBody().get());
    }

    void PhysicsEngine::RemoveRigidBody(Shared<RigidBody> &body)
    {
        s_PhysicsWorldConfiguration->World->removeRigidBody(body->GetBody().get());
        // Note: associated motion state and collision shape are kept in vectors; if you want to free them here,
        // you'd need to track mapping from body -> indices and erase from vectors too.
    }

    Shared<RigidBody> PhysicsEngine::AddRuntimeBody(Entity entity)
    {
        if (!entity.HasComponent<RigidBodyComponent>() || !entity.HasComponent<TransformComponent>())
            return nullptr;

        if (!CollisionBaker::HasShape(entity)) return nullptr;

        auto& transformComponent = entity.GetComponent<TransformComponent>();
        auto& rigidbodyComponent = entity.GetComponent<RigidBodyComponent>();

        RBComplexShape* shape = new RBComplexShape(CollisionBaker::GetShapePath(entity));
        Shared<RigidBody> body = MakeShared<RigidBody>(shape, transformComponent.translation, transformComponent.rotation, transformComponent.scale, rigidbodyComponent.inertia, rigidbodyComponent.mass);

        AddRigidBody(body);
        body->SetUserPointer((void*)(uintptr_t)entity.GetHandle());
        body->SetTransform(transformComponent.translation, transformComponent.rotation);

        rigidbodyComponent.RuntimeBody = body;
        return body;
    }

    void PhysicsEngine::RuntimeStart(Scene* scene)
    {
        s_Scene = scene;
        Init();

		auto view = s_Scene->m_Registry.view<RigidBodyComponent, TransformComponent>();
		for (auto& e : view)
			AddRuntimeBody({ e, s_Scene });
    }

    void PhysicsEngine::RuntimeStop()
    {
		auto view = s_Scene->m_Registry.view<RigidBodyComponent>();
		for (auto e : view)
		{
			Entity entity = { e, s_Scene };
			auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

			Shared<RigidBody> body = rigidbody.RuntimeBody;
			
			if (body)
			{
				RemoveRigidBody(body);
			};
		}

        // Remove rigid bodies from world and delete them (world does not free motion states or collision shapes)
        if (s_PhysicsWorldConfiguration->World) {
            int numObjects = s_PhysicsWorldConfiguration->World->getNumCollisionObjects();
            for (int i = numObjects - 1; i >= 0; --i) {
                btCollisionObject* obj = s_PhysicsWorldConfiguration->World->getCollisionObjectArray()[i];
                btRigidBody* body = btRigidBody::upcast(obj);
                if (body && body->getMotionState()) {
                    // motion states we created are stored in motionStates vector and will be freed automatically
                }
                s_PhysicsWorldConfiguration->World->removeCollisionObject(obj);
                delete obj; // Note: if you added motion state and shape ownership elsewhere, be careful. Here we take simple approach.
            }
        }

        Shutdown();
    }

    void PhysicsEngine::OnUpdate(TimeStep ts)
    {
        StepSimulation(ts);
        CheckCollisions(); // Rethink in case of multithreading

		auto physics = s_Scene->m_Registry.view<RigidBodyComponent, TransformComponent>();
		for (auto& e : physics)
		{
			auto [rb, transform] = physics.get<RigidBodyComponent, TransformComponent>(e);
            if (!rb.RuntimeBody) continue;
            rb.RuntimeBody->GetTransform(transform.translation, transform.rotation);
            transform.IsDirty = true;
		}
    }

    RaycastResult PhysicsEngine::Raycast(const glm::vec3 &origin, const glm::vec3 &direction, float distance)
    {
        btVector3 rayFrom = GlmToBullet(origin);
        btVector3 rayTo = GlmToBullet(origin + direction * distance);

        btCollisionWorld::ClosestRayResultCallback callback(rayFrom, rayTo);
        
        // Optional: Set collision masks to ignore triggers or the player
        // callback.m_collisionFilterMask = ... 

        s_PhysicsWorldConfiguration->World->rayTest(rayFrom, rayTo, callback);

        if (callback.hasHit()) {
            // Retrieve the Entity pointer stored in the Bullet UserPointer
            entt::entity handle = (entt::entity)(uintptr_t)callback.m_collisionObject->getUserPointer();
            Entity* entity = new Entity(handle, s_Scene);
            return { true, entity, BulletToGlm(callback.m_hitPointWorld), BulletToGlm(callback.m_hitNormalWorld) };
        }

        return { false, nullptr, glm::vec3(0.0f), glm::vec3(0.0f) };
    }

}