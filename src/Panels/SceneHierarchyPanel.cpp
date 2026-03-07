#include "SceneHierarchyPanel.h"
#include "../Scene/Entity.h"
#include "../Scene/Components.h"
#include "AssimpLoader.h"
#include "ModelLibrary.h"
#include "RigidBody.h"

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

		m_Context->m_Registry.view<entt::entity>().each([this](entt::entity entity)
		{
			Entity e { entity, m_Context.get() };
			DrawEntityNode(e);
		});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			m_SelectionContext = {};
		}

		if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create entity"))
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
	};

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = m_Context->m_Registry.get<TagComponent>(entity.GetHandle());
		unsigned int selected = m_SelectionContext == entity ? ImGuiTreeNodeFlags_Selected : 0;
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | selected;

		bool opened = ImGui::TreeNodeEx((void*) entity.GetHandle(), flags, tag.tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete entity"))
			{
				entityDeleted = true;
			}
			ImGui::EndPopup();
		}

		if (opened)
		{
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
			{
				m_SelectionContext = {};
			}
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

			if (ImGui::MenuItem("Model"))
			{
				m_SelectionContext.AddComponent<ModelComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Transform"))
			{
				glm::vec3 trans = glm::vec3(0.0f, 0.0f, 0.0f);
				m_SelectionContext.AddComponent<TransformComponent>(trans);
				ImGui::CloseCurrentPopup();
			}

			if (entity.HasComponent<ModelComponent>())
			{
				if (ImGui::MenuItem("RigidBody"))
				{
					// Fix this
					m_SelectionContext.AddComponent<RigidBodyComponent>();
					ModelComponent modelComponent = entity.GetComponent<ModelComponent>();
					TransformComponent transformComponent = entity.GetComponent<TransformComponent>();
					RigidBodyComponent rigidbodyComponent = entity.GetComponent<RigidBodyComponent>();
					Shared<Model> m = std::static_pointer_cast<Model>(AssetManager::LoadAsset(modelComponent.path));
					std::vector<Shared<Mesh>> meshes = m->GetMeshes();
					RBComplexShape* shape = new RBComplexShape(meshes); // Heavy to calculate
					RigidBody* body = new RigidBody(shape, transformComponent.translation, transformComponent.rotation, rigidbodyComponent.inertia, rigidbodyComponent.mass);
					rigidbodyComponent.runtimeBody = body;
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		DrawComponent<ModelComponent>("Model", entity, [entity](auto &component)
		{
			ImGui::Button("Model", ImVec2(100.0f, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
			    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MODEL"))
			    {
				const char* p = (char*)payload->Data;
				std::string path = std::string(p);
				std::cout << p << std::endl;
				component.path = path;
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
		DrawComponent<RigidBodyComponent>("RigidBody", entity, [](auto &component)
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

			RigidBody* body = (RigidBody*) component.runtimeBody;
			if (body)
			{
				float mass = component.mass;

				if (ImGui::DragFloat("Mass", &mass))
				{
					component.mass = mass;
					body->SetMass(mass);
				}
			}
		});
	};

};
