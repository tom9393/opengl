#include "ViewportPanel.h"
#include "../Asset/AssetManager.h"
#include "../Asset/PrimitiveBuilder.h"
#include "../Core/Input.h"
#include "../Core/KeyCodes.h"
#include "../Utils/ScopedImGuiStyle.h"
#include "../Utils/GizmoManager.h"
#include "../Events/EventBroker.h"

namespace Engine {

    ViewportPanel::ViewportPanel()
    {
        EventBroker::Get().Subscribe<MouseButtonPressedEvent>(BIND_FN(OnMouseButtonPressed), this);
    }

    ViewportPanel::~ViewportPanel()
    {
        EventBroker::Get().UnsubscribeAll(this);
    }

    void ViewportPanel::SetContext(Shared<Scene> scene) {
        m_Context = scene;
    }

    void ViewportPanel::SetRenderer(Shared<SceneRenderer> renderer) {
        m_SceneRenderer = renderer;
    }

    void ViewportPanel::SetCamera(Shared<EditorCamera> camera) {
        m_EditorCamera = camera;
    }

    void ViewportPanel::SetSceneDroppedCallback(const std::function<void(const std::string&)>& callback) {
        m_SceneDroppedCallback = callback;
    }

    void ViewportPanel::OnImGuiRender() {
        ImGuiWindowFlags window_flags = 0;
        if (Input::IsKeyPressed(KEY_LEFT_CONTROL))
            window_flags |= ImGuiWindowFlags_NoMove;

        ScopedImGuiStyle noPadding(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
        ImGui::Begin("Viewport", nullptr, window_flags);

        m_IsHovered = ImGui::IsWindowHovered();
        m_IsFocused = ImGui::IsWindowFocused();

        if (m_EditorCamera)
            m_EditorCamera->SetViewportHovered(m_IsHovered);

        // Calculate view bounds and mouse position
        auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
        auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
        auto viewportOffset = ImGui::GetWindowPos();
        m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
        m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        glm::vec2 newViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        if (m_ViewportSize != newViewportSize && newViewportSize.x > 0 && newViewportSize.y > 0) {
            m_ViewportSize = newViewportSize;
            m_EditorCamera->SetViewportSize((unsigned int)m_ViewportSize.x, (unsigned int)m_ViewportSize.y);
            m_SceneRenderer->OnViewportResize((unsigned int)m_ViewportSize.x, (unsigned int)m_ViewportSize.y);
        }

        // Render Framebuffer
        unsigned int texID = m_SceneRenderer->GetRenderGraph().GetPassColorAttachmentRendererID("ScenePass", 0);
        if (texID != 0) {
            ImGui::Image((void*)(intptr_t)texID, ImVec2{m_ViewportSize.x, m_ViewportSize.y}, ImVec2{0, 1}, ImVec2{1, 0});
        }

        DrawDragAndDrop();
        DrawContextMenu();

        if (SceneManager::GetSceneState() == SceneState::Edit) {
            UpdateSelectedEntityGizmo();
        }

        ImGui::End();
    }

    void ViewportPanel::DrawDragAndDrop() {
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetType::Scene")) {
                const char* path = (char*)payload->Data;
                if (m_SceneDroppedCallback) {
                    m_SceneDroppedCallback(path);
                }
            }
            ImGui::EndDragDropTarget();
        }
    }

    void ViewportPanel::DrawContextMenu() {
        if (ImGui::BeginPopupContextWindow("SceneHierarchyContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            // You can add other standard options here too, like "Create Empty Entity"
            
            // Your 3D Object Menu
            if (ImGui::BeginMenu("Create 3D Object"))
            {
                for (auto primitive: AllPrimitives)
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
                        metadata.FilePath = "";
                        metadata.IsMemoryAsset = true;
                        metadata.Type = AssetType::Mesh;

                        Shared<Asset> primitive = PrimitiveBuilder::CreatePrimitive(spec, name);

                        Shared<Material> material = MakeShared<Material>();
                        material->TargetShader = ShaderLibrary::Get("Texture");

                        AssetMetadata materialMeta;
                        materialMeta.Handle = UUID();
                        materialMeta.Name = "Primitive material";
                        materialMeta.FilePath = "";
                        materialMeta.IsMemoryAsset = true;
                        materialMeta.Type = AssetType::Material;

                        UUID meshID = AssetManager::RegisterAsset(metadata, primitive);
                        UUID materialID = AssetManager::RegisterAsset(materialMeta, primitive);

                        entity.AddComponent<MeshComponent>(meshID);
                        entity.AddComponent<TransformComponent>(); 
                        entity.AddComponent<MaterialComponent>(materialID);
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::EndPopup();
        }
    }

    entt::entity ViewportPanel::RaycastSelection() {
        if (!m_IsHovered) return entt::null;

        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;
        
        int mouseX = (int)mx;
        int mouseY = (int)(m_ViewportSize.y - my - 1);
        float width = m_ViewportSize.x;
        float height = m_ViewportSize.y;

        // Ensure we are inside bounds
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

    void ViewportPanel::UpdateSelectedEntityGizmo() {
        Entity selectedEntity = { SceneManager::GetSelectedEntity(), SceneManager::GetContext() };
        if (!selectedEntity || !selectedEntity.HasComponent<TransformComponent>())
            return;

        auto& tc = selectedEntity.GetComponent<TransformComponent>();
        glm::mat4 transform = tc.GetTransform();

        GizmoManager::BeginFrame();
        GizmoManager::SetViewport(m_ViewportBounds[0], m_ViewportBounds[1] - m_ViewportBounds[0]);

        bool snap = Input::IsKeyPressed(KEY_LEFT_SHIFT);
        auto result = GizmoManager::Manipulate(*m_EditorCamera, transform, snap);

        if (result.IsUsing) {
            tc.ApplyMatrix(result.Transform);
        }
    }

    bool ViewportPanel::OnMouseButtonPressed(MouseButtonPressedEvent &e)
    {
        // Handle raycast picking only if clicking in the viewport bounds
        if (e.GetMouseButton() == MOUSE_BUTTON_LEFT && IsHovered() && !ImGuizmo::IsOver())
        {
            entt::entity clickedId = RaycastSelection();
            if (clickedId != entt::null)
                SceneManager::SetSelectedEntity(clickedId);
        }

        return false;
    }
}