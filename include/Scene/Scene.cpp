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

			auto view = m_Registry.view<RigidBodyComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				auto& transform = entity.GetComponent<TransformComponent>();

				btRigidBody* rigidBody = (btRigidBody*)entity.GetComponent<RigidBodyComponent>().runtimeBody;

				btTransform btTransform;
				rigidBody->getMotionState()->getWorldTransform(btTransform);
				btVector3 pos = btTransform.getOrigin();

				transform.translation.x = (float)pos.x();
				transform.translation.y = (float)pos.y();
				transform.translation.z = (float)pos.z();
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
		m_PhysicsWorld = new PhysicsWorld(btVector3(0, -9.81f, 0));
		m_PhysicsWorld->createStaticPlane(btVector3(0, 1, 0), 0.0f);

		auto view = m_Registry.view<RigidBodyComponent, TransformComponent, ModelComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
			auto& modelComp = entity.GetComponent<ModelComponent>();

			// 1. Get the mesh data from your model library
			// This is a placeholder; replace with your actual model retrieval logic
			glm::mat4 t = transform.GetTransform();
			auto meshes = m_ModelLibrary.GetModelByPath(modelComp.model, (int)entity)->GetMeshes();

			// auto model = ModelLibrary::Get(modelComp.model); 
			// auto& meshes = model->GetMeshes();

			btTransform btTrans;
			btTrans.setIdentity();
			btTrans.setOrigin({transform.translation.x, transform.translation.y, transform.translation.z});

			// Use the first mesh to create the collision hull
			auto& mesh = meshes[0]; 

			// 2. Create the convex hull using the mesh vertices
			// Note: You need the 'stride' from your Mesh's BufferLayout
			uint32_t stride = mesh->GetLayout().GetStride() / sizeof(float);

			btRigidBody* body = m_PhysicsWorld->createConvexHullRigidBody(
				800.0f, 
				mesh->GetVertices(), 
				stride, 
				btTrans
			);

			rigidbody.runtimeBody = body;
		}
	}

	void Scene::OnRuntimeStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
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
