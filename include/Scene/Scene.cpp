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
	auto group = m_Registry.group<ModelComponent>(entt::get<TransformComponent>);
	for (auto entity : group)
	{
		auto [model, transform] = group.get<ModelComponent, TransformComponent>(entity);
		auto meshes = model.model->GetMeshes();
		for (auto& mesh : meshes)
		{
			Renderer::SetTexture(mesh);
			Renderer::Submit(mesh, transform.transform);
		}
	}
};

Entity Scene::CreateEntity()
{
	Entity e = { m_Registry.create(), this };
	return e;
};
