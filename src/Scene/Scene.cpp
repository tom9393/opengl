#include "Scene.h"
#include "../Render/Renderer.h"
#include "AssetManager.h"
#include "CollisionBaker.h"
#include "Components.h"
#include "Entity.h"
#include "Model.h"
#include "RigidBody.h"
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
		
		newScene->m_ViewportHeight = other->m_ViewportHeight;
		newScene->m_ViewportWidth = other->m_ViewportWidth;
		
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
		
		CopyComponent<TagComponent>(destination, source, map);
		CopyComponent<TransformComponent>(destination, source, map);
		CopyComponent<ModelComponent>(destination, source, map);
		CopyComponent<SceneCameraComponent>(destination, source, map);
		CopyComponent<RigidBodyComponent>(destination, source, map);
		CopyComponent<NativeScriptComponent>(destination, source, map);
		CopyComponent<LuaScriptComponent>(destination, source, map);

		return newScene;
	};

	void Scene::OnUpdate(TimeStep ts)
	{
		m_Physicsworld.StepSimulation(ts);
		m_Physicsworld.CheckCollisions();

		auto physics = m_Registry.view<RigidBodyComponent, TransformComponent>();
		for (auto& e : physics)
		{

			auto [rb, transform] = physics.get<RigidBodyComponent, TransformComponent>(e);
			if (auto* body = static_cast<RigidBody*>(rb.runtimeBody))
				body->GetTransform(transform.translation, transform.rotation);
		}

		auto scripts = m_Registry.view<NativeScriptComponent>();
		for (auto& e : scripts)
		{
			Entity entity = { e, this };
			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			if (nsc.instance)
				nsc.instance->OnUpdate(ts);
		}

		auto luaScripts = m_Registry.view<LuaScriptComponent>();
		for (auto entity : luaScripts)
		{
			auto& script = luaScripts.get<LuaScriptComponent>(entity);
			
			sol::function onUpdate = script.Environment["OnUpdate"];
			if (onUpdate.valid()) {
				onUpdate(ts.GetSeconds()); // Pass delta time to Lua
			} else {
				std::cout << "Something went wrong" << std::endl;
			}
		}
	};

	void Scene::OnViewportResize(unsigned int width, unsigned int height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		auto view = m_Registry.view<SceneCameraComponent>();
		for (auto& e : view)
		{
			auto& cameraComponent = view.get<SceneCameraComponent>(e);
			if (!cameraComponent.fixedAspectRatio) cameraComponent.sceneCamera->SetViewportSize(width, height);
		}
	};

	void Scene::OnRuntimeStart()
	{	
        EventBroker::Get().Subscribe<MouseButtonPressedEvent>(BIND_FN(OnMousePress), this);
		auto view = m_Registry.view<RigidBodyComponent, TransformComponent, ModelComponent>();
		for (auto& e : view)
		{

			Entity entity = { e, this };
			auto& transformComponent = entity.GetComponent<TransformComponent>();
			auto& rigidbodyComponent = entity.GetComponent<RigidBodyComponent>();
			auto& modelComponent = entity.GetComponent<ModelComponent>();

			modelComponent.model = std::static_pointer_cast<Model>(AssetManager::LoadAsset(modelComponent.path));

			// 1. Ensure a bake exists (or bake it now if missing)
			std::string bakePath = modelComponent.path + ".pmcol";
			if (!std::filesystem::exists(bakePath)) {
			    CollisionBaker::Bake(modelComponent.path, modelComponent.model->GetMeshes());
			}

			RBComplexShape* shape = new RBComplexShape(modelComponent.path);
			RigidBody* body = new RigidBody(shape, transformComponent.translation, transformComponent.rotation, rigidbodyComponent.inertia, rigidbodyComponent.mass);
			if (!body) return;

			rigidbodyComponent.runtimeBody = body;

			m_Physicsworld.AddRigidBody(body);
			body->GetBody()->setUserPointer((void*)(uintptr_t)entity.GetHandle());
			body->SetTransform(transformComponent.translation, transformComponent.rotation);
		}

		auto scripts = m_Registry.view<NativeScriptComponent>();
		for (auto& e : scripts)
		{
			Entity entity = { e, this };
			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			nsc.InstantiateScript(&nsc);
			nsc.instance->m_Entity = e;
			nsc.instance->m_Scene = this;
			nsc.instance->OnCreate();
		}

		ScriptEngine::Init();

		auto luaScripts = m_Registry.view<LuaScriptComponent>();

		for (auto entity : luaScripts) {
			auto& script = luaScripts.get<LuaScriptComponent>(entity);
			
			if (!script.Environment.valid()) {
				auto& lua = ScriptEngine::GetState();
				
				// Create a unique environment for this entity
				script.Environment = sol::environment(lua, sol::create, lua.globals());
				
				// Pass the entity's components into the Lua environment
				// (This allows Lua to say: `Transform.Translation.x = 5`)
				script.Environment["Transform"] = &m_Registry.get<TransformComponent>(entity);
				
				// Load the script file into this specific environment
				lua.script_file(script.FilePath, script.Environment);
				
				// Call OnCreate if it exists
				sol::function onCreate = script.Environment["OnCreate"];
				sol::function onClick = script.Environment["OnClick"];
				if (onCreate.valid()) {
					onCreate();
				}
			}
		}

		m_Physicsworld.SetScene(this);

		m_Physicsworld.RegisterCollisionCallback([this](void* ptrA, void* ptrB, glm::vec3 &hitPos, glm::vec3 &normal){
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
		auto view = m_Registry.view<RigidBodyComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

			RigidBody* body = (RigidBody*)rigidbody.runtimeBody;
			
			if (body)
			{
				m_Physicsworld.RemoveRigidBody(body);
				delete body;
			};
		}

		auto scripts = m_Registry.view<NativeScriptComponent>();
		for (auto e : scripts)
		{
			Entity entity = { e, this };
			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			nsc.instance->OnDestroyed();
			nsc.DestroyScript(&nsc);
		}

		auto luaScripts = m_Registry.view<LuaScriptComponent>();
		for (auto& e : luaScripts) {
			auto& script = luaScripts.get<LuaScriptComponent>(e);
			script.Environment = sol::environment(); 
		}

		ScriptEngine::Shutdown();

		EventBroker::Get().UnsubscribeAll(this);
		m_Physicsworld.RegisterCollisionCallback(nullptr);
	}

	void Scene::ReloadScripts()
	{
		auto& lua = ScriptEngine::GetState();
		auto view = m_Registry.view<LuaScriptComponent>();

		for (auto entity : view) {
			auto& script = view.get<LuaScriptComponent>(entity);

			if (script.Environment.valid() && !script.FilePath.empty()) {
				// 1. Re-run the file into the EXISTING environment
				auto result = lua.script_file(script.FilePath, script.Environment);
				
				if (result.valid()) {
					std::cout << "Reloaded: " << script.FilePath << std::endl;
					
					// 2. Optionally re-call OnCreate to reset state
					sol::protected_function onCreate = script.Environment["OnCreate"];
					if (onCreate.valid()) {
						sol::set_environment(script.Environment, onCreate);
						onCreate();
					}
				} else {
					sol::error err = result;
					std::cerr << "Reload Error: " << err.what() << std::endl;
				}
			}
		}
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

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	};

	void Scene::OnMousePress(MouseButtonPressedEvent &e)
	{
		auto view = m_Registry.view<LuaScriptComponent>();
		for (auto entity : view) {
			auto& script = view.get<LuaScriptComponent>(entity);

			std::cout << "In the native" << std::endl;
			
			sol::protected_function luaOnClick = script.Environment["OnClick"];
			if (luaOnClick.valid()) {
				luaOnClick();
			} else {
				std::cout << "Invalid" << std::endl;
			}
		}
	};
};
