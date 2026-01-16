#include "Scene.h"
#include "../Render/Renderer.h"
#include "Components.h"
#include "Entity.h"
#include "../AssimpLoader.h"

namespace Engine
{

	Scene::Scene() : m_ModelLibrary(new AssimpLoader()) {};

	Scene::~Scene()
	{
	};

	void Scene::OnUpdate(TimeStep ts)
	{
		auto sceneView = m_Registry.view<SceneCameraComponent>();
		for (auto entity : sceneView)
		{
			auto camera = sceneView.get<SceneCameraComponent>(entity);
			if (camera.primary)
			{
				Renderer::BeginScene(camera.sceneCamera->GetProjection());
				break;
			}
		}

		{
			m_PhysicsWorld->stepSimulation(ts);

			auto view = m_Registry.view<RigidBodyComponent, TransformComponent>();
			for (auto e : view)
			{
				auto [rb, transform] = view.get<RigidBodyComponent, TransformComponent>(e);

				if (rb.runtimeBody)
				{
					m_PhysicsWorld->GetTransform((btRigidBody*)rb.runtimeBody, transform.translation, transform.rotation);
				}
			}

		}

		auto group = m_Registry.group<ModelComponent>(entt::get<TransformComponent>);
		for (auto entity : group)
		{
			auto [model, transform] = group.get<ModelComponent, TransformComponent>(entity);
			glm::mat4 t = transform.GetTransform();
			auto meshes = m_ModelLibrary.GetModelByPath(model.model, (int)entity)->GetMeshes();
			for (auto& mesh : meshes)
			{
				Renderer::SetTexture(mesh->GetTextureLayout());
				Renderer::Submit(mesh, t);
			}
		}

		Renderer::EndScene();
	};

	void Scene::OnUpdateEditor(TimeStep ts, Shared<EditorCamera> &camera)
	{
		Renderer::BeginScene(camera->GetViewProjection());
		auto group = m_Registry.group<ModelComponent>(entt::get<TransformComponent>);
		for (auto entity : group)
		{
			auto [model, transform] = group.get<ModelComponent, TransformComponent>(entity);
			glm::mat4 t = transform.GetTransform();
			auto meshes = m_ModelLibrary.GetModelByPath(model.model, (int)entity)->GetMeshes();
			for (auto& mesh : meshes)
			{
				Renderer::SetTexture(mesh->GetTextureLayout());
				Renderer::Submit(mesh, t);
			}
		}

		Renderer::EndScene();
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
		if (m_PhysicsWorld == nullptr)
		{
			m_PhysicsWorld = new PhysicsWorld(btVector3(0, -9.81f, 0));
			m_PhysicsWorld->createStaticPlane({0, 1, 0}, 0.0f);
		}

		auto view = m_Registry.view<RigidBodyComponent, TransformComponent, ModelComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
			auto& modelComp = entity.GetComponent<ModelComponent>();

			glm::mat4 t = transform.GetTransform();
			auto meshes = m_ModelLibrary.GetModelByPath(modelComp.model, (int)entity)->GetMeshes();

			btRigidBody* body = (btRigidBody*)rigidbody.runtimeBody;
			
			if (body != nullptr)
			{
				m_PhysicsWorld->SetTransform(body, transform.translation, transform.rotation, rigidbody.weight);
			} else {
				rigidbody.runtimeBody = m_PhysicsWorld->createModelRigidBody(rigidbody.weight, meshes, transform.translation, transform.rotation);
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
