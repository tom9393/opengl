#pragma once

#include "../../pch.h"
#include "../../Scene/Entity.h"
#include "DrawTransform.h"
#include "DrawMesh.h"
#include "DrawMaterial.h"
#include "DrawCamera.h"
#include "DrawRigidBody.h"
#include "DrawLight.h"
#include "DrawLua.h"

namespace Engine {

	template <typename T, typename Fn>
	inline static void DrawComponent(const std::string &name, Entity entity, Fn fn)
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

    inline static void DrawComponents(Entity &entity)
    {
        DrawComponent<TransformComponent>("TransformComponent", entity, DrawTransform);
        DrawComponent<MeshComponent>("MeshComponent", entity, DrawMesh);
        DrawComponent<MaterialComponent>("TransformComponent", entity, DrawMaterial);

        DrawComponent<SceneCameraComponent>("CameraComponent", entity, DrawCamera);
        DrawComponent<RigidBodyComponent>("RigidbodyComponent", entity, DrawRigidBody);
        DrawComponent<LightComponent>("LightComponent", entity, DrawLight);
        DrawComponent<LuaScriptComponent>("LuaComponent", entity, DrawLua);
    }
}