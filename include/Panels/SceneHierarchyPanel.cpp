#include "SceneHierarchyPanel.h"
#include "../Scene/Entity.h"
#include "../Scene/Components.h"

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
			auto &transform = entity.GetComponent<TransformComponent>().transform;
			ImGui::DragFloat3("Position", glm::value_ptr(transform[3]), 0.1f);

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
