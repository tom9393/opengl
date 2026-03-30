#include "Scene.h"
#include "../Render/Renderer.h"
#include "AssetManager.h"
#include "CollisionBaker.h"
#include "Components.h"
#include "Entity.h"
#include "Model.h"
#include "RigidBody.h"

namespace Engine
{

	template <typename Component>
	static void CopyComponent(entt::registry &destination, entt::registry &source, const std::unordered_map<UUID, entt::entity> &map)
	{
		auto view = source.view<Component>();
		for (auto& e : view)
		{
			UUID id = source.get<IDComponent>(e).id;
			entt::entity entityId = map.at(id);
			auto& component = source.get<Component>(e);
			destination.emplace_or_replace<Component>(entityId, component);
		}
	};

	Shared<Scene> Scene::Copy(Shared<Scene> other)
	{
		Shared<Scene> newScene = MakeShared<Scene>();
		
		newScene->m_ViewportHeight = other->m_ViewportHeight;
		newScene->m_ViewportWidth = other->m_ViewportWidth;
		
		std::unordered_map<UUID, entt::entity> map;

		auto& source = other->m_Registry;
		auto& destination = newScene->m_Registry;

		auto view = source.view<IDComponent>();
		for (auto& e : view)
		{
			UUID id = source.get<IDComponent>(e).id;
			Entity newEntity = newScene->CreateEntityWithUUID(id);
			
			map[id] = newEntity;
		}
		
		CopyComponent<TagComponent>(destination, source, map);
		CopyComponent<TransformComponent>(destination, source, map);
		CopyComponent<ModelComponent>(destination, source, map);
		CopyComponent<SceneCameraComponent>(destination, source, map);
		CopyComponent<RigidBodyComponent>(destination, source, map);
		CopyComponent<NativeScriptComponent>(destination, source, map);

		return newScene;
	};

	void Scene::OnUpdate(TimeStep ts)
	{
		m_Physicsworld.StepSimulation(ts);
		m_Physicsworld.CheckCollisions();

		auto physics = m_Registry.view<RigidBodyComponent, TransformComponent>();
		for (auto& e : physics)
		{

			auto [rb, transform] = physics.get<RigidBodyComponent, TransformComponent>(e);
			if (auto* body = static_cast<RigidBody*>(rb.runtimeBody))
				body->GetTransform(transform.translation, transform.rotation);
		}

		auto scripts = m_Registry.view<NativeScriptComponent>();
		for (auto& e : scripts)
		{
			Entity entity = { e, this };
			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			if (nsc.instance)
				nsc.instance->OnUpdate(ts);
		}
	};

	void Scene::OnViewportResize(unsigned int width, unsigned int height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		auto view = m_Registry.view<SceneCameraComponent>();
		for (auto& e : view)
		{
			auto& cameraComponent = view.get<SceneCameraComponent>(e);
			if (!cameraComponent.fixedAspectRatio) cameraComponent.sceneCamera->SetViewportSize(width, height);
		}
	};

	void Scene::OnRuntimeStart()
	{	
		auto view = m_Registry.view<RigidBodyComponent, TransformComponent, ModelComponent>();
		for (auto& e : view)
		{

			Entity entity = { e, this };
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

			m_Physicsworld.AddRigidBody(body);
			body->GetBody()->setUserPointer((void*)(uintptr_t)entity.GetHandle());
			body->SetTransform(transformComponent.translation, transformComponent.rotation);
		}

		auto scripts = m_Registry.view<NativeScriptComponent>();
		for (auto& e : scripts)
		{
			Entity entity = { e, this };
			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			nsc.InstantiateScript(&nsc);
			nsc.instance->m_Entity = e;
			nsc.instance->m_Scene = this;
			nsc.instance->OnCreate();
		}

		m_Physicsworld.SetScene(this);

		m_Physicsworld.RegisterCollisionCallback([this](void* ptrA, void* ptrB, glm::vec3 &hitPos, glm::vec3 &normal){
                entt::entity a = (entt::entity)(uintptr_t)ptrA;
                entt::entity b = (entt::entity)(uintptr_t)ptrB;

				Entity collider = Entity { a, this };

				if (collider.HasComponent<NativeScriptComponent>())
				{
					auto& nsc = collider.GetComponent<NativeScriptComponent>();
					if (nsc.instance)
					{
						Entity other = { b, this };

						CollisionData data;
						data.OtherPointer = ptrB;
						data.HitPoint = hitPos;
						data.Normal = normal;
						data.Impulse = 20.0f;

						nsc.instance->OnCollision(data); 
					}
				}
				});
	}

	void Scene::OnRuntimeStop()
	{
		auto view = m_Registry.view<RigidBodyComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

			RigidBody* body = (RigidBody*)rigidbody.runtimeBody;
			
			if (body)
			{
				m_Physicsworld.RemoveRigidBody(body);
				delete body;
			};
		}

		auto scripts = m_Registry.view<NativeScriptComponent>();
		for (auto e : scripts)
		{
			Entity entity = { e, this };
			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			nsc.instance->OnDestroyed();
			nsc.DestroyScript(&nsc);
		}
		m_Physicsworld.RegisterCollisionCallback(nullptr);
	}

	Entity Scene::CreateEntity(const std::string &name)
	{
		return CreateEntityWithUUID(UUID(), name);
	};

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string &name)
	{
		Entity e = { m_Registry.create(), this };
		e.AddComponent<TagComponent>(name);
		e.AddComponent<IDComponent>(uuid);
		return e;
	};

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	};
};
