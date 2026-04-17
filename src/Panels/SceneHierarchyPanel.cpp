#include "SceneHierarchyPanel.h"
#include "../Scene/Entity.h"
#include "../Scene/Components.h"
#include "../Asset/CollisionBaker.h"
#include "../Asset/Asset.h"
#include "../Scripting/PlayerScript.h"
#include "../Utils/SceneManager.h"
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
				selection.AddComponent<MaterialComponent>();
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

		DrawComponent<MeshComponent>("Mesh", entity, [entity](auto &component)
		{
			ImGui::Button("Mesh", ImVec2(100.0f, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
			    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetType::Mesh"))
			    {
					UUID handle = *(UUID*)payload->Data;
					component.MeshHandle = handle;
			    }
			    ImGui::EndDragDropTarget();
			}

			if (ImGui::TreeNodeEx("Mesh Attributes", ImGuiTreeNodeFlags_DefaultOpen))
			{
				// 1. Fetch the mesh from the AssetManager using the component's handle
				Shared<Mesh> mesh = AssetManager::GetAsset<Mesh>(component.MeshHandle);
				
				if (mesh)
				{
					auto vertexArray = mesh->GetVAO(); 
					auto indexBuffer = vertexArray->GetIndexBuffer();

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					// 2. Fetch ALL Vertex Buffers
					const auto& vertexBuffers = vertexArray->GetVertexBuffers();
					
					// This tracks the OpenGL layout(location = X) index across ALL buffers
					uint32_t globalAttributeIndex = 0; 

					// 3. Iterate through every VBO
					for (size_t i = 0; i < vertexBuffers.size(); i++)
					{
						auto& vertexBuffer = vertexBuffers[i];
						
						// Push a unique ID for this specific VBO block so ImGui doesn't mix up the tables
						ImGui::PushID((int)i); 
						
						// Create a collapsible header for each VBO
						if (ImGui::TreeNodeEx("VBO_Node", ImGuiTreeNodeFlags_DefaultOpen, "Vertex Buffer %zu", i))
						{
							if (ImGui::BeginTable("BufferLayoutTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
							{
								ImGui::TableSetupColumn("Active", ImGuiTableColumnFlags_WidthFixed, 40.0f);
								ImGui::TableSetupColumn("Name");
								ImGui::TableSetupColumn("Offset");
								ImGui::TableSetupColumn("Size");
								ImGui::TableHeadersRow();

								auto& layout = vertexBuffer->GetLayout(); 

								for (auto& element : layout)
								{
									ImGui::TableNextRow();
									ImGui::TableSetColumnIndex(0);
									
									// Push the global attribute index as the ID for the checkbox
									ImGui::PushID(globalAttributeIndex);
									
									if (ImGui::Checkbox("##enabled", const_cast<bool*>(&element.Enabled)))
									{
										vertexArray->Bind();
										if (element.Enabled) {
											vertexBuffer->EnableAttribute(globalAttributeIndex);
										} else {
											vertexBuffer->DisableAttribute(globalAttributeIndex);
										}
										vertexArray->Unbind();
									}
									
									ImGui::PopID(); // Pop Checkbox ID

									ImGui::TableSetColumnIndex(1);
									ImGui::Text("%s", element.Name.c_str());

									ImGui::TableSetColumnIndex(2);
									ImGui::Text("%d", element.Offset);

									ImGui::TableSetColumnIndex(3);
									ImGui::Text("%d bytes", element.Size);

									// Increment layout location for the next attribute!
									globalAttributeIndex++;
								}
								ImGui::EndTable();
							}
							ImGui::TreePop(); // Pop VBO TreeNode
						}
						ImGui::PopID(); // Pop VBO Block ID
						ImGui::Spacing();
					}
				}
				else
				{
					ImGui::TextDisabled("No Mesh Loaded");
				}
				ImGui::TreePop(); // Pop main Mesh Attributes TreeNode
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
			    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetType::Material"))
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
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetType::Texture2D"))
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
			    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetType::LuaScript"))
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
		DrawComponent<LightComponent>("Light", entity, [](auto &component)
		{
            const char* lightTypeStrings[] = { "Point", "Directional", "Cone" };
            const char* currentLightTypeString = lightTypeStrings[(int)component.Type];

            if (ImGui::BeginCombo("Type", currentLightTypeString))
            {
                for (int i = 0; i < 3; i++)
                {
                    bool isSelected = currentLightTypeString == lightTypeStrings[i];
                    if (ImGui::Selectable(lightTypeStrings[i], isSelected))
                    {
                        currentLightTypeString = lightTypeStrings[i];
                        component.Type = (LightComponent::LightType)i;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            // Color
            glm::vec3 color = component.Color;
            DrawVec3("Color", color, 0.0f);
            component.Color = color;

            // Intensity
            ImGui::DragFloat("Intensity", &component.Intensity, 0.1f, 0.0f, 10000.0f);

            // Conditionally draw Cone properties
            if (component.Type == LightComponent::LightType::Cone)
            {
                // Note: These are stored as cosines, so the values generally range from 0.0 to 1.0
                ImGui::DragFloat("Inner CutOff", &component.InnerCutOff, 0.01f, 0.0f, 1.0f);
                ImGui::DragFloat("Outer CutOff", &component.OuterCutOff, 0.01f, 0.0f, 1.0f);
            }
		});
	};

};
