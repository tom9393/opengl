#include "SceneHierarchyPanel.h"
#include "../../Scene/Entity.h"
#include "../../Scene/Scene.h"
#include "../../Scene/Components.h"
#include "../../Asset/CollisionBaker.h"
#include "../../Asset/Asset.h"
#include "../../Scripting/PlayerScript.h"
#include "../../Utils/SceneManager.h"
#include "DrawComponent.h"

namespace Engine
{

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
        bool hasChildren = entity.HasComponent<RelationshipComponent>() && !entity.GetComponent<RelationshipComponent>().Children.empty();

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

				Entity rootEntity = modelScene->FindRootEntity();
				if (rootEntity) 
				{
					Entity spawnedModel = Entity::CloneEntity(rootEntity, context, selected);
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

};
