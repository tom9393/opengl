#include "Scene.h"
#include "Asset/AssetManager.h"
#include "Asset/CollisionBaker.h"
#include "Asset/Prefab.h"
#include "Audio/AudioEngine.h"
#include "Components.h"
#include "Entity.h"
#include "Events/MouseEvent.h"
#include "Events/EventBroker.h"
#include "ParticleSystem.h"
#include "Physics/RigidBody.h"
#include "Render/Renderer.h"
#include "Scripting/LuaBindings.h"
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

	template <typename... Component>
	static void CopyComponents(ComponentGroup<Component...>, entt::registry &destination, entt::registry &source, const std::unordered_map<UUID, entt::entity> &map)
	{
		(CopyComponent<Component>(destination, source, map), ...);
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
		CopyComponents(AllComponents{}, destination, source, map);

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
		ParticleEngine::OnUpdate(this, ts);
	};

	// Calls the matching lifecycle function on an entity's Lua script, if it defines one.
	// Enter events pass (otherEntity, contactPoint, normal); exit events pass (otherEntity).
	static void DispatchContactToLua(Scene* scene, entt::entity self, entt::entity other, const glm::vec3& point, const glm::vec3& normal, bool entered, bool trigger)
	{
		// A body can despawn mid-overlap, leaving a stale handle in the exit set.
		if (!scene->GetRegistry().valid(self) || !scene->GetRegistry().valid(other)) return;

		Entity e{ self, scene };
		if (!e.HasComponent<LuaScriptComponent>()) return;

		auto& env = e.GetComponent<LuaScriptComponent>().Environment;
		if (!env.valid()) return;

		const char* fn = trigger ? (entered ? "OnTriggerEnter" : "OnTriggerExit")
		                         : (entered ? "OnCollisionEnter" : "OnCollisionExit");

		sol::protected_function callback = env[fn];
		if (!callback.valid()) return;

		Entity otherEntity{ other, scene };
		if (entered)
			callback(otherEntity, point, normal);
		else
			callback(otherEntity);
	}

	// Calls OnCollision on an entity's native script for a contact enter.
	static void DispatchCollisionToNative(Scene* scene, entt::entity self, entt::entity other, const glm::vec3& point, const glm::vec3& normal, float impulse)
	{
		if (!scene->GetRegistry().valid(self)) return;

		Entity e{ self, scene };
		if (!e.HasComponent<NativeScriptComponent>()) return;

		auto& nsc = e.GetComponent<NativeScriptComponent>();
		if (!nsc.instance) return;

		CollisionData data{ other, point, normal, impulse };
		nsc.instance->OnCollision(data);
	}

	void Scene::OnRuntimeStart()
	{
        EventBroker::Get().Subscribe<MouseButtonPressedEvent>(BIND_FN(OnMousePress), this);

		// The play-mode copy shallow-shares the editor's particle systems; reset so
		// play gets fresh state and the editor preview stays untouched.
		auto emitters = m_Registry.view<ParticleEmitterComponent>();
		for (auto e : emitters)
			emitters.get<ParticleEmitterComponent>(e).Runtime = nullptr;

		PhysicsEngine::RuntimeStart(this);
		AudioEngine::RuntimeStart(this);
		ScriptEngine::RuntimeStart(this);

		// Contact enter/exit events -> Lua lifecycle functions and native scripts,
		// dispatched to both entities of the pair with the normal flipped for B.
		PhysicsEngine::RegisterContactCallback([this](const ContactEvent& e){
				DispatchContactToLua(this, e.A, e.B, e.Point, e.Normal, e.Entered, e.Trigger);
				DispatchContactToLua(this, e.B, e.A, e.Point, -e.Normal, e.Entered, e.Trigger);

				// Native scripts only get solid contact enters.
				if (e.Entered && !e.Trigger)
				{
					DispatchCollisionToNative(this, e.A, e.B, e.Point, e.Normal, e.Impulse);
					DispatchCollisionToNative(this, e.B, e.A, e.Point, -e.Normal, e.Impulse);
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

		LuaBindings::CallOnDestroy(entity);

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

		if (entity.HasComponent<RigidBodyComponent>())
		{
			// The component owns the runtime body; pull it out of the Bullet world
			// before the registry destroys it, or the world keeps a dangling pointer.
			auto& rb = entity.GetComponent<RigidBodyComponent>();
			if (rb.RuntimeBody)
				PhysicsEngine::RemoveRigidBody(rb.RuntimeBody);

			CollisionBaker::Release(entity);
		}

		m_Registry.destroy(entity);
	};

	static void CollectSubtree(Entity entity, std::vector<Entity>& out)
	{
		out.push_back(entity);
		for (auto child : entity.GetChildren())
			CollectSubtree(Entity{ child, entity.GetScene() }, out);
	}

	Entity Scene::InstantiatePrefab(Shared<Prefab> prefab, const glm::vec3& position, const glm::vec3& rotation)
	{
		if (!prefab) return {};

		Entity root = prefab->FindRootEntity();
		if (!root) return {};

		Entity spawned = Entity::CloneEntity(root, this, Entity{});

		if (spawned.HasComponent<TransformComponent>())
		{
			auto& transform = spawned.GetComponent<TransformComponent>();
			transform.translation = position;
			transform.rotation = rotation;
			transform.IsDirty = true;
		}

		// Bring the new subtree to life: physics bodies first (scripts read RuntimeBody
		// while binding), then scripts. Only meaningful while the scene is playing.
		std::vector<Entity> subtree;
		CollectSubtree(spawned, subtree);

		for (Entity e : subtree)
			if (e.HasComponent<RigidBodyComponent>())
				PhysicsEngine::AddRuntimeBody(e);

		for (Entity e : subtree)
			if (e.HasComponent<LuaScriptComponent>())
				LuaBindings::BindEntity(e);

		return spawned;
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

	// Currently the assumption is that there is a single root node, that is this function is mostly used to find the root of an imported scene (e.g. .gltf/.fbx)
	Entity Scene::FindRootEntity()
	{
		auto view = m_Registry.view<TagComponent>(entt::exclude<ParentComponent>);
		for (auto e : view)
			return Entity{ e, this };
		return {};
	}
};
