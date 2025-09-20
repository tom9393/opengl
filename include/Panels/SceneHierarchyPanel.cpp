#include "SceneHierarchyPanel.h"
#include "../Scene/Entity.h"
#include "../Scene/Components.h"

template <typename T, typename Fn>
static void DrawComponent(const std::string &name, Entity entity, Fn fn)
{
};

static void DrawVec3(const std::string &label, glm::vec3 &values,
                            float resetValue = 1.0f, float columnWidth = 100.0f)
{
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

        if (ImGui::Button(axisNames[i], buttonSize))
        {
            // toggle selection (click same axis again to deselect)
            selected = (isSelected ? -1 : i);
            // optional: reset when selecting
            if (!isSelected) *axisVals[i] = resetValue;
        }

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
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | selected;

	bool opened = ImGui::TreeNodeEx((void*) entity.GetHandle(), flags, tag.tag.c_str());
	if (ImGui::IsItemClicked())
	{
		m_SelectionContext = entity;
	}

	if (opened)
	{
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
		if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
		{
			tag = std::string(buffer);
		}

	}

	if (entity.HasComponent<TransformComponent>())
	{

		if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
		{
			auto &tc = entity.GetComponent<TransformComponent>();
			glm::vec3 translation = tc.translation;
			glm::vec3 rotation = glm::degrees(tc.rotation);
			glm::vec3 scale = tc.scale;

			// ImGui::DragFloat3("Position", glm::value_ptr(translation), 0.1f);
			
			DrawVec3("Translation", translation, 0.0f);
			DrawVec3("Rotation", rotation, 0.0f);
			DrawVec3("Scale", scale, 0.0f);

			tc.rotation = glm::radians(rotation);

			ImGui::TreePop();
		}
	}

	if (entity.HasComponent<SceneCameraComponent>())
	{

		if (ImGui::TreeNodeEx((void*)typeid(SceneCameraComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Camera"))
		{
			auto &cameraComponent = entity.GetComponent<SceneCameraComponent>();
			auto &camera = cameraComponent.sceneCamera;

			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = projectionTypeStrings[(int)cameraComponent.sceneCamera->GetProjectionType()];

			if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
			{

				for (unsigned int i = 0; i < 2; i++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
					if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
					{
						currentProjectionTypeString = projectionTypeStrings[i];
						cameraComponent.sceneCamera->SetProjectionType((SceneCamera::ProjectionType)i);
					}

					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			bool enabled = cameraComponent.primary;
			if (ImGui::Checkbox("Use", &enabled))
			{
				cameraComponent.primary = !cameraComponent.primary;
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

			ImGui::TreePop();
		}
	}
};
