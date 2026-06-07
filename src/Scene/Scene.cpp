#include "Scene.h"
#include "Asset/AssetManager.h"
#include "Asset/CollisionBaker.h"
#include "Audio/AudioEngine.h"
#include "Components.h"
#include "Entity.h"
#include "Events/MouseEvent.h"
#include "Events/EventBroker.h"
#include "Physics/RigidBody.h"
#include "Render/Renderer.h"
#include "Scripting/ScriptEngine.h"

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
		CopyComponent<LightComponent>(destination, source, map);

		auto parentView = source.view<ParentComponent>();
		for (auto& e : parentView)
		{
			UUID entityUUID = source.get<IDComponent>(e).id;
			entt::entity newEntityId = map.at(entityUUID);
			UUID parentUUID = source.get<IDComponent>(source.get<ParentComponent>(e).Parent).id;
			ParentComponent newParent;
			newParent.Parent = map.at(parentUUID);
			destination.emplace_or_replace<ParentComponent>(newEntityId, newParent);
		}

		auto childrenView = source.view<ChildrenComponent>();
		for (auto& e : childrenView)
		{
			UUID entityUUID = source.get<IDComponent>(e).id;
			entt::entity newEntityId = map.at(entityUUID);
			ChildrenComponent newChildren;
			for (auto childHandle : source.get<ChildrenComponent>(e).Children)
			{
				UUID childUUID = source.get<IDComponent>(childHandle).id;
				newChildren.Children.push_back(map.at(childUUID));
			}
			destination.emplace_or_replace<ChildrenComponent>(newEntityId, newChildren);
		}

		// Share the baked environment (immutable after baking) and post settings.
		newScene->m_Environment = other->m_Environment;
		newScene->m_EnvironmentPath = other->m_EnvironmentPath;
		newScene->m_EnvironmentIntensity = other->m_EnvironmentIntensity;
		newScene->m_PostSettings = other->m_PostSettings;

		return newScene;
	};

	void Scene::SetEnvironmentPath(const std::string& path)
	{
		m_EnvironmentPath = path;
		if (path.empty())
		{
			m_Environment = nullptr;
			return;
		}
		m_Environment = EnvironmentMap::Load(path);
		if (!m_Environment) m_EnvironmentPath.clear();
	}

	void Scene::OnUpdate(TimeStep ts)
	{
		PhysicsEngine::OnUpdate(ts);
		ScriptEngine::OnUpdate(ts);
		AudioEngine::OnUpdate(ts);
	};

	void Scene::OnRuntimeStart()
	{	
        EventBroker::Get().Subscribe<MouseButtonPressedEvent>(BIND_FN(OnMousePress), this);

		PhysicsEngine::RuntimeStart(this);
		AudioEngine::RuntimeStart(this);
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
		AudioEngine::RuntimeStop();
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

		if (entity.HasComponent<ChildrenComponent>())
		{
			auto children = entity.GetComponent<ChildrenComponent>().Children;
			for (auto childHandle : children)
			{
				Entity child{ childHandle, this };
				DestroyEntity(child);
			}
		}

		std::cout << (uint32_t)entity.GetHandle() << std::endl;

		if (entity.HasComponent<RigidBodyComponent>())
			CollisionBaker::Release(entity);

		m_Registry.destroy(entity);
	};

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

	// Currently the assumption is that there is a single root node, that is this function is mostly used to find the root of an imported scene (e.g. .gltf/.fbx)
	Entity Scene::FindRootEntity()
	{
		auto view = m_Registry.view<TagComponent>(entt::exclude<ParentComponent>);
		for (auto e : view)
			return Entity{ e, this };
		return {};
	}
};
