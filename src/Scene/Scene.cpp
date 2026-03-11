#include "Scene.h"
#include "../Render/Renderer.h"
#include "AssetManager.h"
#include "Components.h"
#include "Entity.h"
#include "RigidBody.h"

namespace Engine
{

	template <typename Component>
	static void CopyComponent(entt::registry &destination, entt::registry &source, const std::unordered_map<UUID, entt::entity> &map)
	{
		auto view = source.view<Component>();
		for (auto e : view)
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

		auto ids = source.view<IDComponent>();
		for (auto e : ids)
		{
			UUID id = source.get<IDComponent>(e).id;
			Entity newEntity = newScene->CreateEntityWithUUID(id);
			
			map[id] = newEntity;
		}
		
		CopyComponent<TagComponent>(destination, source, map);
		CopyComponent<TransformComponent>(destination, source, map);
		CopyComponent<ModelComponent>(destination, source, map);
		CopyComponent<SceneCameraComponent>(destination, source, map);
		CopyComponent<NativeScriptComponent>(destination, source, map);
		CopyComponent<RigidBodyComponent>(destination, source, map);

		return newScene;
	};

	void Scene::OnUpdate(TimeStep ts)
	{
		m_Physicsworld.stepSimulation(ts);
		SyncPhysicsTransforms();

		auto view = m_Registry.view<SceneCameraComponent, TransformComponent>(); // Camera must have location
		for (auto entity : view)
		{
			auto [camera, transform] = view.get<SceneCameraComponent, TransformComponent>(entity);
			if (camera.primary)
			{
				glm::mat4 viewMatrix = glm::inverse(transform.GetTransform());
				RenderScene(camera.sceneCamera->GetProjection() * viewMatrix);
				return;
			}
		}
		auto scripts = m_Registry.view<NativeScriptComponent>();
		for (auto e : scripts)
		{
			Entity entity = { e, this };
			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			nsc.instance->OnUpdate(ts);
		}
	};

	void Scene::OnUpdateEditor(TimeStep ts, Shared<EditorCamera> &camera)
	{
		RenderScene(camera->GetViewProjection());
	}

	void Scene::OnViewportResize(unsigned int width, unsigned int height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		auto view = m_Registry.view<SceneCameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<SceneCameraComponent>(entity);
			if (!cameraComponent.fixedAspectRatio) cameraComponent.sceneCamera->SetViewportSize(width, height);
		}
	};

	void Scene::RenderScene(const glm::mat4 &viewProjection)
	{
		Renderer::BeginScene(viewProjection);

		auto group = m_Registry.group<ModelComponent>(entt::get<TransformComponent>);
		for (auto entity : group)
		{
			auto [model, transform] = group.get<ModelComponent, TransformComponent>(entity);

			Shared<Model> m = std::static_pointer_cast<Model>(AssetManager::LoadAsset(model.path));

			if (m) 
				Renderer::SubmitModel(m, transform.GetTransform());
		}

		Renderer::EndScene();
	}

	void Scene::SyncPhysicsTransforms()
	{
		auto view = m_Registry.view<RigidBodyComponent, TransformComponent>();
		for (auto e : view)
		{
			auto [rb, transform] = view.get<RigidBodyComponent, TransformComponent>(e);

			// Use a proper cast and check for the internal body
			if (auto* body = static_cast<RigidBody*>(rb.runtimeBody))
			{
			    body->GetTransform(transform.translation, transform.rotation);
			}
		}
	}

	void Scene::OnRuntimeStart()
	{	
		auto view = m_Registry.view<RigidBodyComponent, TransformComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

			RigidBody* body = (RigidBody*)rigidbody.runtimeBody;
			
			if (body)
			{
				m_Physicsworld.AddRigidBody(body);
				body->SetTransform(transform.translation, transform.rotation);
			};
		}

		auto scripts = m_Registry.view<NativeScriptComponent>();
		for (auto e : scripts)
		{
			Entity entity = { e, this };
			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			nsc.instance->OnCreate();
		}
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
			};
		}

		auto scripts = m_Registry.view<NativeScriptComponent>();
		for (auto e : scripts)
		{
			Entity entity = { e, this };
			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			nsc.instance->OnDestroyed();
		}
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
