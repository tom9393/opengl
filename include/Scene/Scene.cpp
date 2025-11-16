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

	Entity Scene::CreateEntity(const std::string &name)
	{
		Entity e = { m_Registry.create(), this };
		e.AddComponent<TagComponent>(name);
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
	void Scene::OnComponentAdded<EditorCameraComponent>(Entity entity, EditorCameraComponent &component)
	{
		component.editorCamera->SetViewportSize(m_ViewportWidth, m_ViewportHeight);
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

};
