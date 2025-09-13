#include "Scene.h"
#include "../Renderer.h"
#include "Components.h"
#include "Entity.h"
#include "SceneCamera.h"

Scene::Scene()
{
};

Scene::~Scene()
{
};

void Scene::OnUpdate(TimeStep ts)
{
	Shared<EditorCamera> editorCamera = nullptr;
	Shared<SceneCamera> sceneCamera = nullptr;

	auto sceneView = m_Registry.view<SceneCameraComponent>();
	for (auto entity : sceneView)
	{
		auto camera = sceneView.get<SceneCameraComponent>(entity);
		if (camera.primary)
		{
			sceneCamera = camera.sceneCamera;
			break;
		}
	}

	auto editorView = m_Registry.view<EditorCameraComponent>();
	for (auto entity : editorView)
	{
		auto camera = editorView.get<EditorCameraComponent>(entity);
		camera.editorCamera->OnUpdate(ts);
		if (camera.primary)
		{
			editorCamera = camera.editorCamera;
			break;
		}
	}

	if (sceneCamera || editorCamera)
	{
		if (sceneCamera)
		{
			Renderer::BeginScene(sceneCamera->GetProjection());
		}
		else
		{
			Renderer::BeginScene(editorCamera->GetViewProjection());
		}

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
	auto view = m_Registry.view<EditorCameraComponent>();
	for (auto entity : view)
	{
		auto& cameraComponent = view.get<EditorCameraComponent>(entity);
		if (!cameraComponent.fixedAspectRatio) cameraComponent.editorCamera->SetViewportSize(width, height);

	}
};

Entity Scene::CreateEntity()
{
	Entity e = { m_Registry.create(), this };
	return e;
};
