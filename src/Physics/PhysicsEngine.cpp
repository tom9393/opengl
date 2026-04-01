#include "PhysicsEngine.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "RigidBody.h"
#include "../Core/Application.h"
#include "../Scene/Entity.h"
#include "../Utils/ThreadPool.h"
#include "../Asset/AssetManager.h"
#include "../Asset/CollisionBaker.h"

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
    }

    void PhysicsEngine::StepSimulation(float deltaTime, int maxSubSteps, float fixedTimeStep)
    {
        if (!s_PhysicsWorldConfiguration->World) return;
        s_PhysicsWorldConfiguration->World->stepSimulation(deltaTime, maxSubSteps, fixedTimeStep);
    }

    void PhysicsEngine::RenderDebug(const glm::mat4 &viewProjection)
    {
        if (s_PhysicsWorldConfiguration->World && s_PhysicsWorldConfiguration->World->getDebugDrawer())
        {
            s_PhysicsWorldConfiguration->World->debugDrawWorld();
        }
    }

    void PhysicsEngine::CheckCollisions()
    {
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
            glm::vec3 hitPos = BtToGlmVec3(cp.m_normalWorldOnB);
            glm::vec3 normal = BtToGlmVec3(cp.m_normalWorldOnB);
            glm::vec3 flippedNormal = -normal;

            // Notify the Scene/Scripts
            s_Callback(ptrA, ptrB, hitPos, normal);
            s_Callback(ptrB, ptrA, hitPos, flippedNormal);
        }
    }

    void PhysicsEngine::AddRigidBody(RigidBody* body)
    {
        if (!body) return;
        s_PhysicsWorldConfiguration->World->addRigidBody(body->GetBody().get());
    }

    void PhysicsEngine::RemoveRigidBody(RigidBody* body)
    {
        if (!body) return;
        s_PhysicsWorldConfiguration->World->removeRigidBody(body->GetBody().get());
        // Note: associated motion state and collision shape are kept in vectors; if you want to free them here,
        // you'd need to track mapping from body -> indices and erase from vectors too.
    }

    void PhysicsEngine::RuntimeStart(Scene* scene)
    {
        s_Scene = scene;
        Init();

		auto view = s_Scene->m_Registry.view<RigidBodyComponent, TransformComponent, ModelComponent>();
		for (auto& e : view)
		{
			Entity entity = { e, s_Scene };
			auto& transformComponent = entity.GetComponent<TransformComponent>();
			auto& rigidbodyComponent = entity.GetComponent<RigidBodyComponent>();
			auto& modelComponent = entity.GetComponent<ModelComponent>();

			modelComponent.model = std::static_pointer_cast<Model>(AssetManager::LoadAsset(modelComponent.path));

			// 1. Ensure a bake exists (or bake it now if missing)
			std::string bakePath = modelComponent.path + ".pmcol";
			if (!std::filesystem::exists(bakePath)) {
			    CollisionBaker::Bake(modelComponent.path, modelComponent.model->GetMeshes());
			}

			RBComplexShape* shape = new RBComplexShape(modelComponent.path);
			RigidBody* body = new RigidBody(shape, transformComponent.translation, transformComponent.rotation, rigidbodyComponent.inertia, rigidbodyComponent.mass);
			if (!body) return;

			rigidbodyComponent.runtimeBody = body;

			AddRigidBody(body);
			body->GetBody()->setUserPointer((void*)(uintptr_t)entity.GetHandle());
			body->SetTransform(transformComponent.translation, transformComponent.rotation);
		}
    }

    void PhysicsEngine::RuntimeStop()
    {
		auto view = s_Scene->m_Registry.view<RigidBodyComponent>();
		for (auto e : view)
		{
			Entity entity = { e, s_Scene };
			auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

			RigidBody* body = (RigidBody*)rigidbody.runtimeBody;
			
			if (body)
			{
				RemoveRigidBody(body);
				delete body;
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
			if (auto* body = static_cast<RigidBody*>(rb.runtimeBody))
				body->GetTransform(transform.translation, transform.rotation);
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
            Entity* entity = static_cast<Entity*>(callback.m_collisionObject->getUserPointer());
            return { true, entity, BulletToGlm(callback.m_hitPointWorld), BulletToGlm(callback.m_hitNormalWorld) };
        }

        return { false, nullptr, glm::vec3(0.0f), glm::vec3(0.0f) };
    }

}