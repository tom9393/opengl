#include "Scene.h"
#include "../Render/Renderer.h"
#include "AssetManager.h"
#include "Components.h"
#include "Entity.h"
#include "RigidBody.h"
#include <memory>

namespace Engine
{

	void Scene::OnUpdate(TimeStep ts)
	{
		m_PhysicsWorld->stepSimulation(ts);
		SyncPhysicsTransforms();

		auto view = m_Registry.view<SceneCameraComponent, TransformComponent>(); // Camera must have location
		for (auto entity : view)
		{
			auto [camera, transform] = view.get<SceneCameraComponent, TransformComponent>(entity);
			if (camera.primary)
			{
				RenderScene(camera.sceneCamera->GetProjection() * transform.GetTransform());
				return;
			}
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

	void Scene::OnRuntimeStart()
	{
		if (m_PhysicsWorld == nullptr) m_PhysicsWorld = new PhysicsWorld(btVector3(0, -9.81f, 0));

		auto view = m_Registry.view<RigidBodyComponent, TransformComponent, ModelComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
			auto& model = entity.GetComponent<ModelComponent>();

			RigidBody* body = (RigidBody*)rigidbody.runtimeBody;
			
			if (body != nullptr)
			{
				body->SetTransform(transform.translation, transform.rotation);
			} else {
				Shared<Model> m = std::static_pointer_cast<Model>(AssetManager::LoadAsset(model.path));
				auto meshes = m->GetMeshes();
				RBComplexShape* shape = new RBComplexShape(meshes);
				RigidBody* body = new RigidBody(shape, transform.translation, transform.rotation, rigidbody.inertia, rigidbody.mass);
				body->SetWorld(m_PhysicsWorld->getWorld());
				m_PhysicsWorld->AddRigidBody(body);
				rigidbody.runtimeBody = body;
			}
		}
	}

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

	void Scene::OnRuntimeStop()
	{
		// delete m_PhysicsWorld;
		// m_PhysicsWorld = nullptr;
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

	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T &component)
	{
		static_assert(false);
	}

	template <>
	void Scene::OnComponentAdded<SceneCameraComponent>(Entity entity, SceneCameraComponent &component)
	{
		component.sceneCamera->SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template <>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent &component)
	{
	}

	template <>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent &component)
	{
	}

	template <>
	void Scene::OnComponentAdded<ModelComponent>(Entity entity, ModelComponent &component)
	{
	}

	template <>
	void Scene::OnComponentAdded<TextureComponent>(Entity entity, TextureComponent &component)
	{
	}

	template <>
	void Scene::OnComponentAdded<RigidBodyComponent>(Entity entity, RigidBodyComponent &component)
	{
	}

	template <>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent &component)
	{
	}

};
