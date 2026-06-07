#include "ObjectViewportPanel.h"
#include "PassPanel.h"
#include "Asset/Prefab.h"
#include "Asset/AssetManager.h"
#include "Asset/PrimitiveBuilder.h"
#include "Commands/SceneCommands.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Core/MouseButtonCodes.h"
#include "Utils/GizmoManager.h"

namespace Engine {

    void ObjectViewportPanel::OnDraw() {
        if (ImGui::BeginDragDropTarget()) {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetType::Prefab");
            if (!payload) payload = ImGui::AcceptDragDropPayload("AssetType::Scene");
            if (payload)
            {
                UUID handle = *(UUID*)payload->Data;
                Shared<Prefab> prefab = AssetManager::GetAsset<Prefab>(handle);

                Entity rootEntity = prefab->FindRootEntity();
                if (rootEntity)
                {
                    Entity spawnedModel = Entity::CloneEntity(rootEntity, m_Context.get(), Entity{});
                    CommandHistory::Global().Push(MakeUnique<CreateEntityCommand>(spawnedModel));
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::BeginPopupContextWindow("SceneHierarchyContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::BeginMenu("Create 3D Object"))
            {
                for (auto primitive : AllPrimitives)
                {
                    const char* name = PrimitiveNames[static_cast<int>(primitive)];
                    if (ImGui::MenuItem(name))
                    {
                        Entity entity = m_Context->CreateEntity(name);
                        PrimitiveSpecification spec;
                        spec.Type = primitive;
                        spec.Segments[0] = 5;
                        spec.Segments[1] = 5;
                        spec.Segments[2] = 5;
                        spec.Size = glm::vec3(5);

                        AssetMetadata metadata;
                        metadata.Handle = UUID();
                        metadata.Name = name;
                        metadata.FilePath = std::string("../assets/") + name;
                        metadata.IsMemoryAsset = true;
                        metadata.Type = AssetType::Mesh;

                        Shared<Asset> primitiveAsset = PrimitiveBuilder::CreatePrimitive(spec, name);
                        UUID meshID = AssetManager::RegisterAsset(metadata, primitiveAsset);

                        entity.AddComponent<MeshComponent>(meshID);
                        entity.AddComponent<TransformComponent>();

                        CommandHistory::Global().Push(MakeUnique<CreateEntityCommand>(entity));
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::EndPopup();
        }

        if (SceneManager::GetSceneState() == SceneState::Edit) {
            UpdateSelectedEntityGizmo();
        }
    }

    entt::entity ObjectViewportPanel::RaycastSelection() {
        if (!m_IsHovered) return entt::null;

        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;

        int mouseX = (int)mx;
        int mouseY = (int)(m_ViewportSize.y - my - 1);
        float width = m_ViewportSize.x;
        float height = m_ViewportSize.y;

        if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)width && mouseY < (int)height) {
            glm::vec3 rayOrigin = m_EditorCamera->GetPosition();
            glm::mat4 cameraViewMatrix = m_EditorCamera->GetViewMatrix();
            glm::mat4 cameraProjection = m_EditorCamera->GetProjection();

            glm::vec3 rayDir = ScreenPosToWorldRay(mouseX, mouseY, width, height, cameraViewMatrix, cameraProjection);

            entt::entity selectedEntity = entt::null;
            float closestDistance = std::numeric_limits<float>::max();

            auto view = m_Context->GetRegistry().view<TransformComponent, MeshComponent>();
            for (auto& e : view) {
                auto [transformComponent, meshComponent] = view.get<TransformComponent, MeshComponent>(e);
                Shared<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshComponent.MeshHandle);

                float hitDistance;
                if (RayIntersectsAABB(rayOrigin, rayDir, mesh->GetAABB(), transformComponent.GetTransform(), hitDistance)) {
                    if (hitDistance < closestDistance) {
                        closestDistance = hitDistance;
                        selectedEntity = e;
                    }
                }
            }
            return selectedEntity;
        }
        return entt::null;
    }

    void ObjectViewportPanel::UpdateSelectedEntityGizmo() {
        Entity selectedEntity = { SceneManager::GetSelectedEntity(), SceneManager::GetContext() };
        if (!selectedEntity || !selectedEntity.HasComponent<TransformComponent>())
            return;

        auto& tc = selectedEntity.GetComponent<TransformComponent>();
        glm::mat4 transform = tc.GetTransform();

        GizmoManager::BeginFrame();
        GizmoManager::SetViewport(m_ViewportBounds[0], m_ViewportBounds[1] - m_ViewportBounds[0]);

        bool snap = Input::IsKeyPressed(KEY_LEFT_SHIFT);
        auto result = GizmoManager::Manipulate(*m_EditorCamera, transform, snap);

        // Snapshot the transform at drag start, apply during the drag, and record one
        // undo step at drag end (so a whole gizmo manipulation is a single Ctrl+Z).
        if (result.IsUsing && !m_GizmoUsing) {
            m_GizmoEntity = selectedEntity.GetUUID();
            m_GizmoBefore = CaptureEntity(selectedEntity);
        }

        if (result.IsUsing) {
            tc.ApplyMatrix(result.Transform);
        }

        if (!result.IsUsing && m_GizmoUsing) {
            Entity edited = FindEntityByUUID(SceneManager::GetContext(), m_GizmoEntity);
            if (edited) {
                EntitySnapshot after = CaptureEntity(edited);
                if (SnapshotsDiffer(m_GizmoBefore, after))
                    CommandHistory::Global().Push(
                        MakeUnique<ModifyEntityCommand>(m_GizmoEntity, m_GizmoBefore, after));
            }
        }

        m_GizmoUsing = result.IsUsing;
    }

    bool ObjectViewportPanel::OnMousePressed(MouseButtonPressedEvent& e) {
        if (e.GetMouseButton() == MOUSE_BUTTON_LEFT && IsHovered() && !ImGuizmo::IsOver())
        {
            entt::entity clickedId = RaycastSelection();
            if (clickedId != entt::null)
                SceneManager::SetSelectedEntity(clickedId);
        }

        return false;
    }

}
