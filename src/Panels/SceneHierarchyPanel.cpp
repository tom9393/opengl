#include "SceneHierarchyPanel.h"
#include "../Scene/Entity.h"
#include "../Scene/Components.h"
#include "../Asset/CollisionBaker.h"
#include "../Scripting/PlayerScript.h"
#include "imgui.h"

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

	std::string AssetTypeToString(AssetType type)
	{
		switch (type)
		{
			case AssetType::None:      return "AssetType::None";
			case AssetType::Scene:     return "AssetType::Scene";
			case AssetType::Mesh: return "AssetType::Mesh";
			case AssetType::Texture2D: return "AssetType::Texture2D";
			case AssetType::Material: return "AssetType::Material";
			case AssetType::Model: return "AssetType::Model";
		}

		return "AssetType::<Invalid>";
	}

	static void DrawVec3(const std::string &label, glm::vec3 &values, float resetValue = 1.0f, float columnWidth = 100.0f)
	{
		ImGuiIO &io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		// Persist selection per-control (keyed by label)
		static std::unordered_map<std::string, int> s_selectedAxis;

		// ensure a default of -1 (no axis selected)
		if (s_selectedAxis.find(label) == s_selectedAxis.end())
		s_selectedAxis[label] = -1;

		int &selected = s_selectedAxis[label];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::TextUnformatted(label.c_str());
		ImGui::NextColumn();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{4, 4});

		// button sizing
		float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 4.0f, lineHeight };

		const char* axisNames[3] = { "X", "Y", "Z" };
		float* axisVals[3] = { &values.x, &values.y, &values.z };
		ImVec4 axisColors[3] = {
		ImVec4(0.75f, 0.12f, 0.12f, 1.0f), // red-ish X
		ImVec4(0.12f, 0.75f, 0.12f, 1.0f), // green-ish Y
		ImVec4(0.12f, 0.25f, 0.75f, 1.0f)  // blue-ish Z
		};

		// Render three inline axis controls: [Button(X)] [DragFloat]
		for (int i = 0; i < 3; ++i)
		{
		ImGui::PushID(i);

		bool isSelected = (selected == i);

		// color the button if selected (and slightly darker when not)
		ImVec4 baseBtnColor = ImGui::GetStyleColorVec4(ImGuiCol_Button);
		ImVec4 btnColor = isSelected ? axisColors[i] : ImVec4(baseBtnColor.x * 0.7f, baseBtnColor.y * 0.7f, baseBtnColor.z * 0.7f, baseBtnColor.w);

		ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(btnColor.x + 0.08f, btnColor.y + 0.08f, btnColor.z + 0.08f, btnColor.w));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(btnColor.x - 0.08f, btnColor.y - 0.08f, btnColor.z - 0.08f, btnColor.w));
		ImGui::PushFont(boldFont);

		if (ImGui::Button(axisNames[i], buttonSize))
		{
		    // toggle selection (click same axis again to deselect)
		    selected = (isSelected ? -1 : i);
		    // optional: reset when selecting
		    if (!isSelected) *axisVals[i] = resetValue;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();

		// unique ID for the DragFloat. Use hidden label with the main label to avoid collisions.
		char dragLabel[64];
		std::snprintf(dragLabel, sizeof(dragLabel), "##%s_%c", label.c_str(), 'X' + i);

		// set width and render DragFloat
		ImGui::SetNextItemWidth(120.0f);
		ImGui::DragFloat(dragLabel, axisVals[i], 0.1f);

		ImGui::PopID();

		if (i < 2) ImGui::SameLine();
		}

		ImGui::PopStyleVar(); // ItemSpacing
		ImGui::Columns(1);
		ImGui::PopID();
	}

	SceneHierarchyPanel::SceneHierarchyPanel(const Shared<Scene> &context)
	{
		SetContext(context);
	};

	void SceneHierarchyPanel::SetContext(const Shared<Scene> &context)
	{
		m_Context = context;
		m_SelectionContext = {};
	};

	void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
	{
		m_SelectionContext = entity;
	};

	void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene hierarchy");

        // --- NEW: Provide a drop target on the empty window space to un-parent entities ---
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
            {
                entt::entity payloadEntity = *(const entt::entity*)payload->Data;
                Entity child{ payloadEntity, m_Context.get() };
                
                // An empty entity implies "remove parent" in a robust SetParent implementation
                Entity nullParent{}; 
                child.SetParent(nullParent); 
            }
        }

        m_Context->m_Registry.view<entt::entity>().each([this](entt::entity entity)
        {
            Entity e { entity, m_Context.get() };
            
            // --- NEW: Only draw ROOT entities in the main loop ---
            if (!e.HasParent())
            {
                DrawEntityNode(e);
            }
        });

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
        {
            m_SelectionContext = {};
        }

        if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::MenuItem("Create empty entity"))
            {
                m_Context->CreateEntity("Empty entity");
            }
            ImGui::EndPopup();
        }

        ImGui::End();

        ImGui::Begin("Properties");
        if (m_SelectionContext)
        {
            DrawComponents(m_SelectionContext);
        }
        ImGui::End();

		if (m_Deleted)
		{
			m_Context->DestroyEntity(m_SelectionContext);
			m_SelectionContext = {};
			m_Deleted = false;
		}
    };

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& tag = m_Context->m_Registry.get<TagComponent>(entity.GetHandle());
        
        // Check if we have children to determine if we should render as a Leaf or a Tree Node
        bool hasChildren = entity.HasComponent<RelationshipComponent>() && 
                           !entity.GetComponent<RelationshipComponent>().Children.empty();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (m_SelectionContext == entity) flags |= ImGuiTreeNodeFlags_Selected;
        if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;

        bool opened = ImGui::TreeNodeEx((void*) entity.GetHandle(), flags, tag.tag.c_str());
        
        if (ImGui::IsItemClicked())
        {
            m_SelectionContext = entity;
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
                Entity child{ payloadEntity, m_Context.get() };
                
                // Prevent parenting to yourself or your own children (prevents infinite loops!)
                if (child != entity) 
                {
                    child.SetParent(entity);
                }
            }
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MODEL"))
			{
				UUID handle = *(UUID*)payload->Data;
				AssetMetadata metadata = AssetManager::GetMetadata(handle);

				// Iterate through every mesh handle stored in the Model asset
				for (size_t i = 0; i < metadata.Children.size(); i++)
				{
					Entity childEntity = m_Context->CreateEntity("MeshPart_" + std::to_string(i));
					childEntity.SetParent(m_SelectionContext);

					UUID meshHandle = metadata.Children[i].Handle;
					childEntity.AddComponent<MeshComponent>(meshHandle);
					childEntity.AddComponent<TransformComponent>();

					// Attach the Material Components
					for (size_t j = 0; j < metadata.Children[i].Children.size(); j++)
					{
						UUID materialHandle = metadata.Children[i].Children[j].Handle;
						childEntity.AddComponent<MaterialComponent>(materialHandle);
					}
				}
			}
            ImGui::EndDragDropTarget();
        }

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Create child entity"))
            {
                Entity child = m_Context->CreateEntity("Empty entity");
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
                    Entity child{ childID, m_Context.get() };
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

		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("AddComponent");
		}

		if (ImGui::BeginPopup("AddComponent"))
		{

			if (ImGui::MenuItem("Camera"))
			{
				m_SelectionContext.AddComponent<SceneCameraComponent>(MakeShared<SceneCamera>());
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Mesh"))
			{
				m_SelectionContext.AddComponent<MeshComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Transform"))
			{
				glm::vec3 trans = glm::vec3(0.0f, 0.0f, 0.0f);
				m_SelectionContext.AddComponent<TransformComponent>(trans);
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Material"))
			{
				m_SelectionContext.AddComponent<MaterialComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::BeginMenu("NativeScript"))
			{
				if (ImGui::MenuItem("PlayerScript"))
				{
					NativeScriptComponent &nsc = m_SelectionContext.AddComponent<NativeScriptComponent>();
					nsc.Bind<PlayerScript>();
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("LuaScript"))
			{
				m_SelectionContext.AddComponent<LuaScriptComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("RigidBody"))
			{
				m_SelectionContext.AddComponent<RigidBodyComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("PointLight"))
			{
				m_SelectionContext.AddComponent<PointLightComponent>();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		DrawComponent<MeshComponent>("Mesh", entity, [entity](auto &component)
		{
			ImGui::Button("Mesh", ImVec2(100.0f, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
			    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MESH"))
			    {
					UUID handle = *(UUID*)payload->Data;
					component.MeshHandle = handle;
			    }
			    ImGui::EndDragDropTarget();
			}
		});
		DrawComponent<TransformComponent>("Transform", entity, [](auto &component)
		{
			glm::vec3 translation = component.translation;
			glm::vec3 rotation = glm::degrees(component.rotation);
			glm::vec3 scale = component.scale;

			DrawVec3("Translation", translation, 0.0f);
			DrawVec3("Rotation", rotation, 0.0f);
			DrawVec3("Scale", scale, 1.0f);

			component.translation = translation;
			component.rotation = glm::radians(rotation);
			component.scale = scale;

		});
		DrawComponent<MaterialComponent>("MaterialComponent", entity, [entity](auto &component)
		{
			ImGui::Button("Material", ImVec2(100.0f, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
			    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL"))
			    {
					UUID handle = *(UUID*)payload->Data;
					component.MaterialHandle = handle;
			    }
			    ImGui::EndDragDropTarget();
			}
			UUID handle = component.MaterialHandle;
			if (AssetManager::IsAssetLoaded(handle))
			{
				Shared<Material> material = AssetManager::GetAsset<Material>(handle);
				
				ImGui::Separator();
				
				// 1. Core Material Settings
				if (ImGui::TreeNodeEx("Settings", ImGuiTreeNodeFlags_DefaultOpen))
				{
					// Blend Mode Selection
					const char* blendModes[] = { "Opaque", "Transparent", "Additive" };
					int currentMode = (int)material->mode;
					if (ImGui::Combo("Blend Mode", &currentMode, blendModes, IM_ARRAYSIZE(blendModes)))
						material->mode = (BlendMode)currentMode;

					ImGui::Checkbox("Depth Write", &material->DepthWrite);
					ImGui::Checkbox("Double Sided", &material->DoubleSided);
					
					ImGui::TreePop();
				}

				// 2. Uniforms / Parameters
				if (ImGui::TreeNodeEx("Parameters", ImGuiTreeNodeFlags_DefaultOpen))
				{
					// Floats
					for (auto& [name, val] : material->m_Floats)
					{
						ImGui::DragFloat(name.c_str(), &val, 0.05f);
					}

					// Bools
					for (auto& [name, val] : material->m_Bools)
					{
						ImGui::Checkbox(name.c_str(), &val);
					}

					// Vec4s (Usually Colors)
					for (auto& [name, val] : material->m_Vec4s)
					{
						ImGui::ColorEdit4(name.c_str(), glm::value_ptr(val));
					}

					// Textures
					for (auto& [name, tex] : material->m_Textures)
					{
						ImGui::Text("%s", name.c_str());
						ImGui::SameLine();
						
						// Display a small thumbnail or button for the texture
						void* texID = tex ? (void*)(uintptr_t)tex->GetID() : nullptr;
						if (ImGui::ImageButton(name.c_str(), (ImTextureID)texID, ImVec2(64, 64), { 0, 1 }, { 1, 0 }))
						{
							// Logic to select texture asset could go here
						}

						if (ImGui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE2D"))
							{
								UUID texHandle = *(UUID*)payload->Data;
								material->m_Textures[name] = AssetManager::GetAsset<Texture2D>(texHandle);
							}
							ImGui::EndDragDropTarget();
						}
					}

					ImGui::TreePop();
				}
			}
			else
			{
				ImGui::TextDisabled("No Material Assigned (Drag & Drop a Material Asset)");
			}
		});
		DrawComponent<SceneCameraComponent>("Camera", entity, [](auto &component)
		{
			auto &camera = component.sceneCamera;

			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = projectionTypeStrings[(int)camera->GetProjectionType()];

			if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
			{
				for (unsigned int i = 0; i < 2; i++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
					if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
					{
						currentProjectionTypeString = projectionTypeStrings[i];
						camera->SetProjectionType((SceneCamera::ProjectionType)i);
					}

					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			bool enabled = component.primary;
			if (ImGui::Checkbox("Use", &enabled))
			{
				component.primary = !component.primary;
			}

			if (camera->GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				float fov = glm::degrees(camera->GetPerspectiveVerticalFOV());
				if(ImGui::DragFloat("Fov", &fov))
					camera->SetPerspectiveVerticalFOV(glm::radians(fov));

				float near = camera->GetOrthographicNearClip();
				if(ImGui::DragFloat("Near", &near))
					camera->SetOrthographicNearClip(near);

				float far = camera->GetOrthographicFarClip();
				if(ImGui::DragFloat("Far", &far))
					camera->SetOrthographicFarClip(far);
			}

			if (camera->GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				float orthoSize = camera->GetOrthographicSize();
				if(ImGui::DragFloat("Size", &orthoSize))
					camera->SetOrthographicSize(orthoSize);

				float near = camera->GetOrthographicNearClip();
				if(ImGui::DragFloat("Near", &near))
					camera->SetOrthographicNearClip(near);

				float far = camera->GetOrthographicFarClip();
				if(ImGui::DragFloat("Far", &far))
					camera->SetOrthographicFarClip(far);
			}

		});
		DrawComponent<NativeScriptComponent>("NativeScript", entity, [](auto &component)
		{

		});
		DrawComponent<LuaScriptComponent>("LuaScript", entity, [](auto &component)
		{
			if (ImGui::BeginDragDropTarget())
			{
			    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LUASCRIPT"))
			    {
					UUID handle = *(UUID*)payload->Data;
					component.ScriptHandle = handle;
			    }
			    ImGui::EndDragDropTarget();
			}

		});
		DrawComponent<RigidBodyComponent>("RigidBody", entity, [&entity](auto &component)
		{
			const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
			const char* currentBodyTypeString = bodyTypeStrings[(int)component.type];

			if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
			{
				for (unsigned int i = 0; i < 3; i++)
				{
					bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
					if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
					{
						currentBodyTypeString = bodyTypeStrings[i];
						component.type = (RigidBodyComponent::BodyType)i;
					}

					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}
			if (ImGui::Button("Re-Bake Collision Data"))
			{
				std::vector<Shared<Mesh>> meshes;

				std::vector<Entity> meshEntities;
				entity.GetAllChildrenWithComponent<MeshComponent>(meshEntities);
				for (auto& e : meshEntities)
				{
					UUID handle = e.GetComponent<MeshComponent>().MeshHandle;
					meshes.emplace_back(AssetManager::GetAsset<Mesh>(handle));
				}

				AssetMetadata metadata = AssetManager::GetMetadata(meshEntities[0].GetComponent<MeshComponent>().MeshHandle);

				// 1. Ensure a bake exists (or bake it now if missing)
				CollisionBaker::Bake(metadata.FilePath, meshes);
			}
			// Add a small hint so the user knows what this does
			if (ImGui::IsItemHovered())
			    ImGui::SetTooltip("Recalculates the simplified physics hull from the high-poly mesh.");

			if (ImGui::DragFloat("Mass", &component.mass))
			{
			}
		});
		DrawComponent<PointLightComponent>("PointLight", entity, [](auto &component)
		{
			glm::vec3 color = component.Color;

			DrawVec3("Color", color, 0.0f);
			if(ImGui::DragFloat("Intensity", &component.Intensity)) {

			}

			component.Color = color;
		});
	};

};
