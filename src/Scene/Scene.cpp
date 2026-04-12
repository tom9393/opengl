#include "Scene.h"
#include "../Render/Renderer.h"
#include "../Asset/AssetManager.h"
#include "../Asset/CollisionBaker.h"
#include "Components.h"
#include "Entity.h"
#include "../Asset/Model.h"
#include "../Physics/RigidBody.h"
#include "Scripting/ScriptEngine.h"
#include "../Events/MouseEvent.h"
#include "../Events/EventBroker.h"

namespace Engine
{

	template <typename Component>
	static void CopyComponent(entt::registry &destination, entt::registry &source, const std::unordered_map<UUID, entt::entity> &map)
	{
		auto view = source.view<Component>();
		for (auto& e : view)
		{
			UUID id = source.get<IDComponent>(e).id;
			entt::entity entityId = map.at(id);
			auto& component = source.get<Component>(e);
			destination.emplace_or_replace<Component>(entityId, component);
		}
	};

	Shared<Scene> Scene::Copy(Shared<Scene> other)
	{
		Shared<Scene> newScene = MakeShared<Scene>();
		
		std::unordered_map<UUID, entt::entity> map;

		auto& source = other->m_Registry;
		auto& destination = newScene->m_Registry;

		auto view = source.view<IDComponent>();
		for (auto& e : view)
		{
			UUID id = source.get<IDComponent>(e).id;
			Entity newEntity = newScene->CreateEntityWithUUID(id);
			
			map[id] = newEntity;
		}
		
		// Note: all components are being shallow copied
		CopyComponent<TagComponent>(destination, source, map);
		CopyComponent<TransformComponent>(destination, source, map);
		CopyComponent<MeshComponent>(destination, source, map);
		CopyComponent<MaterialComponent>(destination, source, map);
		CopyComponent<TextureComponent>(destination, source, map);
		CopyComponent<SceneCameraComponent>(destination, source, map);
		CopyComponent<RigidBodyComponent>(destination, source, map);
		CopyComponent<NativeScriptComponent>(destination, source, map);
		CopyComponent<LuaScriptComponent>(destination, source, map);
		CopyComponent<PointLightComponent>(destination, source, map);

		auto relView = source.view<RelationshipComponent>();
		for (auto& e : relView)
		{
			UUID entityUUID = source.get<IDComponent>(e).id;
			entt::entity newEntityId = map.at(entityUUID);
			auto& oldRel = source.get<RelationshipComponent>(e);

			RelationshipComponent newRel;
			
			// 1. Translate the Parent handle
			if (oldRel.Parent != entt::null)
			{
				// Find the parent's UUID in the old scene, then get its new handle in the new scene
				UUID parentUUID = source.get<IDComponent>(oldRel.Parent).id;
				newRel.Parent = map.at(parentUUID);
			}

			// 2. Translate all Children handles
			for (auto childHandle : oldRel.Children)
			{
				UUID childUUID = source.get<IDComponent>(childHandle).id;
				newRel.Children.push_back(map.at(childUUID));
			}

			destination.emplace_or_replace<RelationshipComponent>(newEntityId, newRel);
		}

		return newScene;
	};

	void Scene::OnUpdate(TimeStep ts)
	{
		PhysicsEngine::OnUpdate(ts);
		ScriptEngine::OnUpdate(ts);
	};

	void Scene::OnRuntimeStart()
	{	
        EventBroker::Get().Subscribe<MouseButtonPressedEvent>(BIND_FN(OnMousePress), this);

		PhysicsEngine::RuntimeStart(this);
		ScriptEngine::RuntimeStart(this);

		PhysicsEngine::RegisterCollisionCallback([this](void* ptrA, void* ptrB, glm::vec3 &hitPos, glm::vec3 &normal){
                entt::entity a = (entt::entity)(uintptr_t)ptrA;
                entt::entity b = (entt::entity)(uintptr_t)ptrB;

				Entity collider = Entity { a, this };

				if (collider.HasComponent<NativeScriptComponent>())
				{
					auto& nsc = collider.GetComponent<NativeScriptComponent>();
					if (nsc.instance)
					{
						Entity other = { b, this };

						CollisionData data;
						data.OtherPointer = ptrB;
						data.HitPoint = hitPos;
						data.Normal = normal;
						data.Impulse = 20.0f;

						nsc.instance->OnCollision(data); 
					}
				}
				});
	}

	void Scene::OnRuntimeStop()
	{

		ScriptEngine::RuntimeStop();
		PhysicsEngine::RuntimeStop();

		EventBroker::Get().UnsubscribeAll(this);
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

	void Scene::DestroyEntity(Entity& entity)
	{
		if (!entity) return;

		Entity nullparent {};
		entity.SetParent(nullparent);

		if (entity.HasComponent<RelationshipComponent>())
		{
			auto children = entity.GetComponent<RelationshipComponent>().Children;
			for (auto childHandle : children)
			{
				Entity child{ childHandle, this };
				DestroyEntity(child);
			}
		}

		std::cout << (uint32_t)entity.GetHandle() << std::endl;

		m_Registry.destroy(entity);
	};

	void Scene::UpdateWorldTransforms()
	{
		auto view = m_Registry.view<TransformComponent>();
		for (auto e : view)
		{
			Entity entity{ e, this };
			
			// We only want to start the cascade from ROOT entities. 
			// Children will be updated automatically by their parents.
			if (!entity.HasParent())
			{
				// Root entities have no parent, so their "parent matrix" is just Identity (1.0f)
				UpdateTransformTree(e, glm::mat4(1.0f));
			}
		}
	}

	void Scene::UpdateTransformTree(entt::entity handle, const glm::mat4 &parentTransform)
	{
		Entity entity{ handle, this };
		if (!entity.HasComponent<TransformComponent>()) return;

		auto& transformComp = entity.GetComponent<TransformComponent>();
		
		// 1. Get the local offset (translation, rotation, scale)
		glm::mat4 localTransform = transformComp.GetTransform();
		
		// 2. CACHE THE RESULT: Parent's World Matrix * My Local Matrix = My World Matrix
		transformComp.WorldTransform = parentTransform * localTransform;

		// 3. Cascade down to children, passing MY new World Matrix as THEIR Parent Matrix
		if (entity.HasComponent<RelationshipComponent>())
		{
			auto& relComp = entity.GetComponent<RelationshipComponent>();
			for (auto childHandle : relComp.Children)
			{
				UpdateTransformTree(childHandle, transformComp.WorldTransform);
			}
		}
	}

	void Scene::OnMousePress(MouseButtonPressedEvent &e)
	{
		auto view = m_Registry.view<LuaScriptComponent>();
		for (auto entity : view) {
			auto& script = view.get<LuaScriptComponent>(entity);

			sol::protected_function luaOnClick = script.Environment["OnClick"];
			if (luaOnClick.valid()) {
				luaOnClick();
			} else {
				std::cout << "Invalid" << std::endl;
			}
		}
	};
};
