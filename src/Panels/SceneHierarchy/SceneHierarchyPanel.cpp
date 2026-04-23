#include "SceneHierarchyPanel.h"
#include "../../Scene/Entity.h"
#include "../../Scene/Components.h"
#include "../../Asset/CollisionBaker.h"
#include "../../Asset/Asset.h"
#include "../../Scripting/PlayerScript.h"
#include "../../Utils/SceneManager.h"
#include "DrawComponent.h"

namespace Engine
{

	template <typename T, typename Fn>
	static void DrawComponent(const std::string &name, Entity entity, Fn fn)
	{
		const ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

		if (entity.HasComponent<T>())
		{
			auto &component = entity.GetComponent<T>();
			ImVec2 available = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});

			float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), nodeFlags, name.c_str());
			ImGui::Separator();
			ImGui::PopStyleVar();

			ImGui::SameLine(available.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{lineHeight, lineHeight}))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
				{
					removeComponent = true;
				}
				ImGui::EndPopup();
			}

			if (open)
			{
				fn(component);
				ImGui::TreePop();
			}

			if (removeComponent)
			{
				entity.RemoveComponent<T>();
			}
		}
	};

	// Temporarily here
	static Entity CloneEntity(Entity sourceEntity, Scene* targetScene, Entity targetParent)
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
		// Add other components as needed

		auto children = sourceEntity.GetChildren(); 
		for (auto& child : children) 
		{ 
			Entity e { child, sourceEntity.GetScene() };
			CloneEntity(e, targetScene, newEntity);
		}

		return newEntity;
	}

	// Temporarily here
	Entity FindRootEntity(Scene* scene)
	{
		auto view = scene->GetRegistry().view<TagComponent>();
		
		for (auto e : view) 
		{
			Entity entity { e, scene };
			if (!entity.HasParent()) 
			{
				return entity; 
			}
		}
		
		return {};
	}

	// Temporarily here
	static std::string PropertyKeyToString(PropertyKey key)
	{
		switch (key)
		{
			case PropertyKey::AlbedoTint:          return "Albedo Tint";
			case PropertyKey::Emission:            return "Emission";
			case PropertyKey::RoughnessMultiplier: return "Roughness Multiplier";
			case PropertyKey::MetallicMultiplier:  return "Metallic Multiplier";
			default:                               return "Unknown Parameter";
		}
	}

	// Temporarily here
	static std::string TextureTypeToString(TextureType type)
	{
		switch (type)
		{
			case TextureType::Albedo:       return "Albedo Map";
			case TextureType::Normal:       return "Normal Map";
			case TextureType::Metallic:     return "Metallic Map";
			case TextureType::Roughness:    return "Roughness Map";
			case TextureType::AO:           return "Ambient Occlusion";
			case TextureType::Emissive:     return "Emissive Map";
			default:                        return "Texture Map";
		}
	}

	void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene hierarchy");

		Scene* context = SceneManager::GetContext();
		if (!context) return;

        // --- NEW: Provide a drop target on the empty window space to un-parent entities ---
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
            {
                entt::entity payloadEntity = *(const entt::entity*)payload->Data;
                Entity child{ payloadEntity, context };
                
                // An empty entity implies "remove parent" in a robust SetParent implementation
                Entity nullParent{}; 
                child.SetParent(nullParent); 
            }
        }

        context->m_Registry.view<entt::entity>().each([this, &context](entt::entity entity)
        {
            Entity e { entity, context };
            
            // --- NEW: Only draw ROOT entities in the main loop ---
            if (!e.HasParent())
            {
                DrawEntityNode(e);
            }
        });

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
        {
			SceneManager::SetSelectedEntity(entt::null);
        }

        if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::MenuItem("Create empty entity"))
            {
                context->CreateEntity("Empty entity");
            }
            ImGui::EndPopup();
        }

        ImGui::End();

		Entity selection = SceneManager::GetSelectedEntity();

        ImGui::Begin("Properties");
        if (selection)
        {
            DrawComponents(selection);
        }
        ImGui::End();

		if (m_Deleted)
		{
			context->DestroyEntity(selection);
			SceneManager::SetSelectedEntity(entt::null);
			m_Deleted = false;
		}
    };

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
		Scene* context = SceneManager::GetContext();
		Entity selected = SceneManager::GetSelectedEntity();
        auto& tag = context->m_Registry.get<TagComponent>(entity.GetHandle());
        
        // Check if we have children to determine if we should render as a Leaf or a Tree Node
        bool hasChildren = entity.HasComponent<RelationshipComponent>() && 
                           !entity.GetComponent<RelationshipComponent>().Children.empty();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (selected == entity) flags |= ImGuiTreeNodeFlags_Selected;
        if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;

        bool opened = ImGui::TreeNodeEx((void*) entity.GetHandle(), flags, tag.tag.c_str());
        
        if (ImGui::IsItemClicked())
        {
			SceneManager::SetSelectedEntity(entity.GetHandle());
        }

        // --- NEW: Drag and Drop Implementation ---
        if (ImGui::BeginDragDropSource())
        {
            entt::entity entityHandle = entity.GetHandle();
            ImGui::SetDragDropPayload("ENTITY", &entityHandle, sizeof(entt::entity));
            ImGui::Text("Move %s", tag.tag.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
            {
                entt::entity payloadEntity = *(const entt::entity*)payload->Data;
                Entity child{ payloadEntity, context };
                
                // Prevent parenting to yourself or your own children (prevents infinite loops!)
                if (child != entity) 
                {
                    child.SetParent(entity);
                }
            }
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetType::Model"))
			{
				UUID handle = *(UUID*)payload->Data;
				Shared<Scene> modelScene = AssetManager::GetAsset<Scene>(handle);

				Entity rootEntity = FindRootEntity(modelScene.get());
				if (rootEntity) 
				{
					Entity spawnedModel = CloneEntity(rootEntity, context, selected);
					spawnedModel.SetParent(selected);
				}
				else 
				{
					std::cout << "Warning: Could not find a root entity in the imported model scene!\n";
				}
			}
            ImGui::EndDragDropTarget();
        }

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Create child entity"))
            {
                Entity child = context->CreateEntity("Empty entity");
                child.SetParent(entity);
            }
            if (ImGui::MenuItem("Delete entity"))
            {
                m_Deleted = true;
            }
            ImGui::EndPopup();
        }

        // --- NEW: Recursive drawing of children ---
        if (opened)
        {
            if (hasChildren)
            {
                auto& children = entity.GetComponent<RelationshipComponent>().Children;
                for (auto childID : children)
                {
                    Entity child{ childID, context };
                    DrawEntityNode(child);
                }
            }
            ImGui::TreePop();
        }
    };

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy(buffer, tag.c_str());
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) tag = std::string(buffer);
			// Add the Handle next to it
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
			ImGui::Text("[%d]", (uint32_t)entity.GetHandle());
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		Entity selection = SceneManager::GetSelectedEntity();

		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("AddComponent");
		}

		if (ImGui::BeginPopup("AddComponent"))
		{

			if (ImGui::MenuItem("Camera"))
			{
				selection.AddComponent<SceneCameraComponent>(MakeShared<SceneCamera>());
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Mesh"))
			{
				selection.AddComponent<MeshComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Transform"))
			{
				glm::vec3 trans = glm::vec3(0.0f, 0.0f, 0.0f);
				selection.AddComponent<TransformComponent>(trans);
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Material"))
			{
				MaterialComponent& mc = selection.AddComponent<MaterialComponent>();
				mc.MaterialHandle = UUID();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::BeginMenu("NativeScript"))
			{
				if (ImGui::MenuItem("PlayerScript"))
				{
					NativeScriptComponent &nsc = selection.AddComponent<NativeScriptComponent>();
					nsc.Bind<PlayerScript>();
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("LuaScript"))
			{
				selection.AddComponent<LuaScriptComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("RigidBody"))
			{
				selection.AddComponent<RigidBodyComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Light"))
			{
				selection.AddComponent<LightComponent>();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		DrawComponents(entity);

	};

};
