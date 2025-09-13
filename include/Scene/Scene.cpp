#include "Scene.h"
#include "../Renderer.h"
#include "Components.h"
#include "Entity.h"

Scene::Scene()
{
};

Scene::~Scene()
{
};

void Scene::OnUpdate(TimeStep ts)
{
	Shared<EditorCamera> mainCamera = nullptr;

	auto view = m_Registry.view<CameraComponent>();
	for (auto entity : view)
	{
		auto camera = view.get<CameraComponent>(entity);
		camera.sceneCamera->OnUpdate(ts);
		if (camera.primary)
		{
			mainCamera = camera.sceneCamera;
			break;
		}
	}

	if (mainCamera)
	{
		Renderer::BeginScene(mainCamera->GetViewProjection());

		auto group = m_Registry.group<ModelComponent>(entt::get<TransformComponent>);
		for (auto entity : group)
		{
			auto [model, transform] = group.get<ModelComponent, TransformComponent>(entity);
			auto meshes = model.model->GetMeshes();
			for (auto& mesh : meshes)
			{
				Renderer::SetTexture(mesh->GetTextureLayout());
				Renderer::Submit(mesh, transform.transform);
			}
		}

		Renderer::EndScene();
	}
};

void Scene::OnViewportResize(unsigned int width, unsigned int height)
{
	m_ViewportWidth = width;
	m_ViewportHeight = height;
	auto view = m_Registry.view<CameraComponent>();
	for (auto entity : view)
	{
		auto& cameraComponent = view.get<CameraComponent>(entity);
		if (!cameraComponent.fixedAspectRatio) cameraComponent.sceneCamera->SetViewportSize(width, height);

	}
};

Entity Scene::CreateEntity()
{
	Entity e = { m_Registry.create(), this };
	return e;
};
