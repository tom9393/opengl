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
		// 1. Prevent self-parenting
		if (*this == parent)
		{
			std::cout << "Warning: Cannot parent an entity to itself!" << std::endl;
			return;
		}

		// Ensure this entity has a relationship component to store the link
		if (!HasComponent<RelationshipComponent>()) 
		{
			AddComponent<RelationshipComponent>();
		}
		auto& myRelationship = GetComponent<RelationshipComponent>();

		// 2. Detach from current parent (if we have one)
		if (myRelationship.Parent != entt::null)
		{
			Entity oldParent{ myRelationship.Parent, m_Scene };
			if (oldParent && oldParent.HasComponent<RelationshipComponent>())
			{
				auto& oldParentRel = oldParent.GetComponent<RelationshipComponent>();
				
				// Find and remove ourselves from the old parent's children list
				auto it = std::find(oldParentRel.Children.begin(), oldParentRel.Children.end(), m_Handle);
				if (it != oldParentRel.Children.end())
				{
					oldParentRel.Children.erase(it);
				}
			}
		}

		// 3. Attach to new parent
		if (parent) // Assuming your Entity has an `operator bool()` to check if it's a valid handle
		{
			if (!parent.HasComponent<RelationshipComponent>()) 
			{
				parent.AddComponent<RelationshipComponent>();
			}
			auto& parentRelationship = parent.GetComponent<RelationshipComponent>();

			myRelationship.Parent = parent.m_Handle;
			parentRelationship.Children.push_back(m_Handle);
		}
		else
		{
			// If the passed parent is an empty entity, it means we are un-parenting this entity.
			myRelationship.Parent = entt::null;
		}
	}

	bool Entity::HasParent()
	{
		if (!HasComponent<RelationshipComponent>()) return false;
		return GetComponent<RelationshipComponent>().Parent != entt::null;
	}

};
