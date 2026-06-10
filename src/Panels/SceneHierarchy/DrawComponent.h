#pragma once

#include "pch.h"
#include "DrawTransform.h"
#include "DrawMesh.h"
#include "DrawMaterial.h"
#include "DrawCamera.h"
#include "DrawRigidBody.h"
#include "DrawLight.h"
#include "DrawLua.h"
#include "DrawParticleEmitter.h"
#include "Commands/SceneCommands.h"
#include "Scene/Entity.h"

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
				fn(entity, component);
				ImGui::TreePop();
			}

			if (removeComponent)
			{
				if constexpr (std::is_same_v<T, RigidBodyComponent>)
					CollisionBaker::Release(entity);
				RecordEntityEdit(entity, [&]{ entity.RemoveComponent<T>(); });
			}
		}
	};

    inline static void DrawComponents(Entity &entity)
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
				RecordEntityEdit(selection, [&]{ selection.AddComponent<SceneCameraComponent>(MakeShared<SceneCamera>()); });
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Mesh"))
			{
				RecordEntityEdit(selection, [&]{ selection.AddComponent<MeshComponent>(); });
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Transform"))
			{
				glm::vec3 trans = glm::vec3(0.0f, 0.0f, 0.0f);
				RecordEntityEdit(selection, [&]{ selection.AddComponent<TransformComponent>(trans); });
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Material"))
			{
				RecordEntityEdit(selection, [&]{ MaterialComponent& mc = selection.AddComponent<MaterialComponent>();
				mc.MaterialHandle = UUID(); });
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::BeginMenu("NativeScript"))
			{
				if (ImGui::MenuItem("PlayerScript"))
				{
					RecordEntityEdit(selection, [&]{ NativeScriptComponent &nsc = selection.AddComponent<NativeScriptComponent>();
					nsc.Bind<PlayerScript>(); });
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("LuaScript"))
			{
				RecordEntityEdit(selection, [&]{ selection.AddComponent<LuaScriptComponent>(); });
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("RigidBody"))
			{
				RecordEntityEdit(selection, [&]{ selection.AddComponent<RigidBodyComponent>(); });
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Light"))
			{
				RecordEntityEdit(selection, [&]{ selection.AddComponent<LightComponent>(); });
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Particle Emitter"))
			{
				RecordEntityEdit(selection, [&]{ selection.AddComponent<ParticleEmitterComponent>(); });
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

        DrawComponent<TransformComponent>("TransformComponent", entity, DrawTransform);
        DrawComponent<MeshComponent>("MeshComponent", entity, DrawMesh);
        DrawComponent<MaterialComponent>("MaterialComponent", entity, DrawMaterial);
        DrawComponent<SceneCameraComponent>("CameraComponent", entity, DrawCamera);
        DrawComponent<RigidBodyComponent>("RigidbodyComponent", entity, DrawRigidBody);
        DrawComponent<LightComponent>("LightComponent", entity, DrawLight);
        DrawComponent<LuaScriptComponent>("LuaComponent", entity, DrawLua);
        DrawComponent<ParticleEmitterComponent>("ParticleEmitterComponent", entity, DrawParticleEmitter);
    }
}