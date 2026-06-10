#include "SceneHierarchyPanel.h"
#include "../../Scene/Entity.h"
#include "../../Scene/Scene.h"
#include "../../Scene/PrefabSerializer.h"
#include "../../Scene/Components.h"
#include "../../Asset/CollisionBaker.h"
#include "../../Asset/Asset.h"
#include "../../Commands/SceneCommands.h"
#include "../../Scripting/PlayerScript.h"
#include "../../Utils/SceneManager.h"
#include "DrawComponent.h"

namespace Engine
{

	namespace {
		// Coalesces an inspector interaction (a drag, a typed value, etc.) into one undo
		// step. While any ImGui item is active we hold a snapshot taken when it first
		// became active; when the interaction ends we push a single ModifyEntityCommand
		// if anything actually changed. Editing a different panel snapshots the (then
		// unchanged) selection harmlessly.
		void TrackInspectorEdit(Entity selection)
		{
			static bool s_Editing = false;
			static UUID s_Entity = 0;
			static EntitySnapshot s_Before;

			bool active = ImGui::IsAnyItemActive();
			if (active && !s_Editing && selection)
			{
				s_Editing = true;
				s_Entity = selection.GetUUID();
				s_Before = CaptureEntity(selection);
			}
			else if (!active && s_Editing)
			{
				s_Editing = false;
				Entity entity = FindEntityByUUID(SceneManager::GetContext(), s_Entity);
				if (entity)
				{
					EntitySnapshot after = CaptureEntity(entity);
					// Component add/remove (a set change) is recorded at its action site;
					// the tracker only commits in-place value edits.
					if (SameComponentSet(s_Before, after) && SnapshotsDiffer(s_Before, after))
						CommandHistory::Global().Push(
							MakeUnique<ModifyEntityCommand>(s_Entity, s_Before, after));
				}
			}
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
                RecordReparent(child, nullParent);
            }
        }

        context->GetRegistry().view<entt::entity>().each([this, &context](entt::entity entity)
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
                Entity created = context->CreateEntity("Empty entity");
                CommandHistory::Global().Push(MakeUnique<CreateEntityCommand>(created));
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

        TrackInspectorEdit(selection);

		if (m_Deleted)
		{
			if (selection)
				CommandHistory::Global().Execute(MakeUnique<DeleteEntityCommand>(selection));
			SceneManager::SetSelectedEntity(entt::null);
			m_Deleted = false;
		}
    };

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
		Scene* context = SceneManager::GetContext();
		Entity selected = SceneManager::GetSelectedEntity();
        auto& tag = context->GetRegistry().get<TagComponent>(entity.GetHandle());
        
        // Check if we have children to determine if we should render as a Leaf or a Tree Node
        bool hasChildren = entity.HasComponent<ChildrenComponent>() && !entity.GetComponent<ChildrenComponent>().Children.empty();

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
                    RecordReparent(child, entity);
                }
            }
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetType::Model"))
			{
				UUID handle = *(UUID*)payload->Data;
				Shared<Scene> modelScene = AssetManager::GetAsset<Scene>(handle);

				Entity rootEntity = modelScene->FindRootEntity();
				if (rootEntity)
				{
					Entity spawnedModel = Entity::CloneEntity(rootEntity, context, selected);
					spawnedModel.SetParent(selected);
					CommandHistory::Global().Push(MakeUnique<CreateEntityCommand>(spawnedModel));
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
                CommandHistory::Global().Push(MakeUnique<CreateEntityCommand>(child));
            }
            if (ImGui::MenuItem("Delete entity"))
            {
                m_Deleted = true;
            }
            if (ImGui::MenuItem("Save as prefab"))
            {
                std::string name = entity.GetComponent<TagComponent>().tag;
                std::string path = "../assets/" + name + ".prefab";
                PrefabSerializer::Serialize(entity, path);
                AssetManager::ImportAsset(path);
            }
            ImGui::EndPopup();
        }

        // --- NEW: Recursive drawing of children ---
        if (opened)
        {
            if (hasChildren)
            {
                auto& children = entity.GetComponent<ChildrenComponent>().Children;
                for (auto childID : children)
                {
                    Entity child{ childID, context };
                    DrawEntityNode(child);
                }
            }
            ImGui::TreePop();
        }
    };

};
