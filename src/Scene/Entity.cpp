#include "Entity.h"

namespace Engine
{

	Entity::Entity(entt::entity entity, Scene* scene) : m_Handle(entity), m_Scene(scene)
	{};

	entt::entity Entity::GetParent()
	{
		if (!HasComponent<RelationshipComponent>()) return entt::null;
		return GetComponent<RelationshipComponent>().Parent;
	}

	std::vector<entt::entity> Entity::GetChildren()
	{
		if (!HasComponent<RelationshipComponent>()) return std::vector<entt::entity>();
		return GetComponent<RelationshipComponent>().Children;

	}

	void Entity::SetParent(Entity &parent)
	{
		// 1. Ensure both entities have RelationshipComponents
		if (!HasComponent<RelationshipComponent>()) AddComponent<RelationshipComponent>();
		if (!parent.HasComponent<RelationshipComponent>()) parent.AddComponent<RelationshipComponent>();

		auto& myRelationship = GetComponent<RelationshipComponent>();
		auto& parentRelationship = parent.GetComponent<RelationshipComponent>();

		// 2. Set the parent
		myRelationship.Parent = parent.m_Handle;

		// 3. Add ourselves to the parent's children list
		parentRelationship.Children.push_back(m_Handle);
	}

	bool Entity::HasParent()
	{
		if (!HasComponent<RelationshipComponent>()) return false;
		return GetComponent<RelationshipComponent>().Parent != entt::null;
	}

};
