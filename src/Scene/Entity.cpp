#include "Entity.h"

namespace Engine
{

	Entity::Entity(entt::entity entity, Scene* scene) : m_Handle(entity), m_Scene(scene)
	{};

	entt::entity Entity::GetParent()
	{
		if (!HasComponent<ParentComponent>()) return entt::null;
		return GetComponent<ParentComponent>().Parent;
	}

	std::vector<entt::entity> Entity::GetChildren()
	{
		if (!HasComponent<ChildrenComponent>()) return {};
		return GetComponent<ChildrenComponent>().Children;
	}


	Entity Entity::CloneEntity(Entity sourceEntity, Scene* targetScene, Entity targetParent)
	{
		std::string name = sourceEntity.GetComponent<TagComponent>().tag;
		Entity newEntity = targetScene->CreateEntity(name);

		if (targetParent) { 
			newEntity.SetParent(targetParent);
		}

		if (sourceEntity.HasComponent<TransformComponent>()) {
			newEntity.AddComponent<TransformComponent>(sourceEntity.GetComponent<TransformComponent>());
		}
		
		if (sourceEntity.HasComponent<MeshComponent>()) {
			newEntity.AddComponent<MeshComponent>(sourceEntity.GetComponent<MeshComponent>());
		}

		if (sourceEntity.HasComponent<MaterialComponent>()) {
			newEntity.AddComponent<MaterialComponent>(sourceEntity.GetComponent<MaterialComponent>());
		}

		if (sourceEntity.HasComponent<SceneCameraComponent>()) {
			auto& srcCamera = sourceEntity.GetComponent<SceneCameraComponent>();
			// Deep copy
			auto& newCamera = newEntity.AddComponent<SceneCameraComponent>(MakeShared<SceneCamera>(*srcCamera.sceneCamera));
			newCamera.primary = srcCamera.primary;
			newCamera.fixedAspectRatio = srcCamera.fixedAspectRatio;
		}

		if (sourceEntity.HasComponent<LightComponent>()) {
			newEntity.AddComponent<LightComponent>(sourceEntity.GetComponent<LightComponent>());
		}

		// RuntimeBody is rebuilt when the body is added to the world.
		if (sourceEntity.HasComponent<RigidBodyComponent>()) {
			newEntity.AddComponent<RigidBodyComponent>(sourceEntity.GetComponent<RigidBodyComponent>());
		}

		// The environment is created fresh when the script is bound.
		if (sourceEntity.HasComponent<LuaScriptComponent>()) {
			newEntity.AddComponent<LuaScriptComponent>(sourceEntity.GetComponent<LuaScriptComponent>());
		}

		auto children = sourceEntity.GetChildren(); 
		for (auto& child : children) 
		{ 
			Entity e { child, sourceEntity.GetScene() };
			CloneEntity(e, targetScene, newEntity);
		}

		return newEntity;
	}

	void Entity::SetParent(Entity &parent)
	{
		if (*this == parent)
		{
			std::cout << "Warning: Cannot parent an entity to itself!" << std::endl;
			return;
		}

		// Detach from current parent if we have one
		if (HasComponent<ParentComponent>())
		{
			Entity oldParent{ GetComponent<ParentComponent>().Parent, m_Scene };
			RemoveComponent<ParentComponent>();

			if (oldParent && oldParent.HasComponent<ChildrenComponent>())
			{
				auto& children = oldParent.GetComponent<ChildrenComponent>().Children;
				auto it = std::find(children.begin(), children.end(), m_Handle);
				if (it != children.end())
					children.erase(it);

				if (children.empty())
					oldParent.RemoveComponent<ChildrenComponent>();
			}
		}

		// Attach to new parent
		if (parent)
		{
			AddComponent<ParentComponent>().Parent = parent.m_Handle;

			if (!parent.HasComponent<ChildrenComponent>())
				parent.AddComponent<ChildrenComponent>();
			parent.GetComponent<ChildrenComponent>().Children.push_back(m_Handle);
		}
	}

	bool Entity::HasParent()
	{
		return HasComponent<ParentComponent>();
	}

};
