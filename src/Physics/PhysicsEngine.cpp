#include "PhysicsEngine.h"
#include "Asset/AssetManager.h"
#include "Asset/CollisionBaker.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "Core/Application.h"
#include "PhysicsDebugDrawer.h"
#include "RigidBody.h"
#include "Scene/Entity.h"
#include "Utils/MathTools.h"
#include "Utils/ThreadPool.h"
#include <bullet/src/btBulletDynamicsCommon.h>

namespace Engine {

    struct PhysicsWorldConfiguration
    {
        std::unique_ptr<btDefaultCollisionConfiguration> CollisionConfig;
        std::unique_ptr<btCollisionDispatcher> Dispatcher;
        std::unique_ptr<btDbvtBroadphase> Broadphase;
        std::unique_ptr<btSequentialImpulseConstraintSolver> Solver;
        std::unique_ptr<btDiscreteDynamicsWorld> World;
        std::unique_ptr<PhysicsDebugDrawer> Debug;
    };

    std::unique_ptr<PhysicsWorldConfiguration> PhysicsEngine::s_PhysicsWorldConfiguration;
    Scene* PhysicsEngine::s_Scene;
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
        s_Scene = nullptr;
        s_Contacts.Clear();
    }

    btDiscreteDynamicsWorld* PhysicsEngine::GetWorld()
    {
        return s_PhysicsWorldConfiguration->World.get();
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

    void PhysicsEngine::AddRigidBody(Shared<RigidBody> &body)
    {
        s_PhysicsWorldConfiguration->World->addRigidBody(body->GetBody().get());
    }

    void PhysicsEngine::RemoveRigidBody(Shared<RigidBody> &body)
    {
        s_PhysicsWorldConfiguration->World->removeRigidBody(body->GetBody().get());
    }

    // World matrix of the entity's parent chain (identity for root entities).
    static glm::mat4 ParentWorldMatrix(Entity entity)
    {
        glm::mat4 world(1.0f);
        entt::entity parent = entity.GetParent();
        while (parent != entt::null)
        {
            Entity p { parent, entity.GetScene() };
            if (!p.HasComponent<TransformComponent>()) break;
            world = p.GetComponent<TransformComponent>().GetTransform() * world;
            parent = p.GetParent();
        }
        return world;
    }

    Shared<RigidBody> PhysicsEngine::AddRuntimeBody(Entity entity)
    {
        if (!entity.HasComponent<RigidBodyComponent>() || !entity.HasComponent<TransformComponent>())
            return nullptr;

        if (!CollisionBaker::HasShape(entity)) return nullptr;

        auto& transformComponent = entity.GetComponent<TransformComponent>();
        auto& rigidbodyComponent = entity.GetComponent<RigidBodyComponent>();

        // Bodies simulate in world space; bake the parent chain into the spawn pose
        glm::vec3 translation, rotation, scale;
        MathTools::DecomposeMatrix(ParentWorldMatrix(entity) * transformComponent.GetTransform(), translation, rotation, scale);

        RBComplexShape* shape = new RBComplexShape(CollisionBaker::GetShapePath(entity));

        // Only dynamic bodies simulate with mass; static and kinematic are fixed (mass 0) in Bullet
        float mass = rigidbodyComponent.type == RigidBodyComponent::BodyType::Dynamic ? rigidbodyComponent.mass : 0.0f;
        Shared<RigidBody> body = MakeShared<RigidBody>(shape, translation, rotation, scale, rigidbodyComponent.inertia, mass);

        if (rigidbodyComponent.type == RigidBodyComponent::BodyType::Kinematic)
            body->SetKinematic(true);

        AddRigidBody(body);
        body->SetUserPointer(EntityToUserPointer(entity.GetHandle()));

        rigidbodyComponent.RuntimeBody = body;
        return body;
    }

    void PhysicsEngine::RuntimeStart(Scene* scene)
    {
        s_Scene = scene;
        Init();

		auto view = s_Scene->GetRegistry().view<RigidBodyComponent, TransformComponent>();
		for (auto& e : view)
			AddRuntimeBody({ e, s_Scene });
    }

    void PhysicsEngine::RuntimeStop()
    {
		auto view = s_Scene->GetRegistry().view<RigidBodyComponent>();
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

        Shutdown();
    }

    void PhysicsEngine::OnUpdate(TimeStep ts)
    {
		auto physics = s_Scene->GetRegistry().view<RigidBodyComponent, TransformComponent>();

		// Kinematic bodies are driven by their transform; push world poses before stepping so the solver sees them this frame
		for (auto& e : physics)
		{
			auto [rb, transform] = physics.get<RigidBodyComponent, TransformComponent>(e);
            if (!rb.RuntimeBody || rb.type != RigidBodyComponent::BodyType::Kinematic) continue;

            glm::vec3 translation, rotation, scale;
            MathTools::DecomposeMatrix(ParentWorldMatrix({ e, s_Scene }) * transform.GetTransform(), translation, rotation, scale);
            rb.RuntimeBody->SetWorldTransform(translation, glm::quat(rotation));
		}

        StepSimulation(ts);
        s_Contacts.Update(s_PhysicsWorldConfiguration->Dispatcher.get()); // Rethink in case of multithreading

		for (auto& e : physics)
		{
			auto [rb, transform] = physics.get<RigidBodyComponent, TransformComponent>(e);
            if (!rb.RuntimeBody || rb.type != RigidBodyComponent::BodyType::Dynamic) continue;
            if (!rb.RuntimeBody->IsActive()) continue; // sleeping bodies haven't moved

            glm::vec3 worldPos;
            glm::quat worldRot;
            rb.RuntimeBody->GetWorldTransform(worldPos, worldRot);

            Entity entity { e, s_Scene };
            if (entity.GetParent() != entt::null)
            {
                // Bring the world-space pose back into the parent's space, scale is left untouched as the simulation never changes it
                glm::mat4 local = glm::inverse(ParentWorldMatrix(entity)) * glm::translate(glm::mat4(1.0f), worldPos) * glm::toMat4(worldRot);
                glm::vec3 translation, rotation, scale;
                MathTools::DecomposeMatrix(local, translation, rotation, scale);
                transform.translation = translation;
                transform.rotation = rotation;
            }
            else
            {
                transform.translation = worldPos;
                transform.rotation = glm::eulerAngles(worldRot);
            }
            transform.IsDirty = true;
		}
    }

}
