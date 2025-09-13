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
