#include "Entity.h"

namespace Engine
{

	Entity::Entity(entt::entity entity, Scene* scene) : m_Handle(entity), m_Scene(scene)
	{};

};
