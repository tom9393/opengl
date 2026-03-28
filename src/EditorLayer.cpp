#include "../pch.h"
#include "AssetManager.h"
#include "MouseButtonCodes.h"
#include "EditorLayer.h"
#include "Render/Renderer.h"
#include "Input.h"
#include "KeyCodes.h"
#include "Application.h"
#include "Scene/Entity.h"
#include "Scene/SceneSerializer.h"
#include "Utils/PlatformUtils.h"
#include "Utils/ScopedImGuiStyle.h"
#include "Scene/Components.h"
#include "Events/EventBroker.h"

namespace Engine
{

    EditorLayer::EditorLayer()
        : m_RenderGraph(500, 500)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        io.Fonts->AddFontFromFileTTF("../assets/fonts/opensans/OpenSans-Bold.ttf", 18.0f);
        io.FontDefault = io.Fonts->AddFontFromFileTTF("../assets/fonts/opensans/OpenSans-Regular.ttf", 18.0f);

        m_EditorCamera = MakeShared<EditorCamera>(90.0f, 2.0f, 0.1f, 1000.0f);
        m_EditingScene = MakeShared<Scene>();
        m_SceneRenderer = MakeShared<SceneRenderer>(m_EditingScene.get());
        m_EditingScene->OnViewportResize(500, 500);
        m_SceneHierarchyPanel.SetContext(m_EditingScene);

        auto& pass = m_RenderGraph.AddPass("ScenePass");
        pass.SetAttachments({ FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth });
        pass.SetClearColor({ 0.2f, 0.2f, 0.2f, 1.0f });
        pass.SetClearInt(1, -1);

        // Callback will be called each frame (with the frame's TimeStep)
        pass.SetCallback([this](TimeStep ts)
        {
            switch (m_SceneState) {
                case Engine::SceneState::Edit:
                {
                    m_EditingScene->OnUpdate(ts);
                    m_SceneRenderer->RenderEditor(m_EditorCamera->GetViewProjection(), m_EditorCamera->GetPosition());
                    break;
                }
                case Engine::SceneState::Play:
                {
                    m_RuntimeScene->OnUpdate(ts);
                    m_SceneRenderer->RenderRuntime();
                    break;
                }
            }
        });
    };

    EditorLayer::~EditorLayer()
    {
        EventBroker::Get().UnsubscribeAll(this);
    };

    void EditorLayer::OnAttach() {
        std::cout << "EditorLayer attached" << std::endl;
        EventBroker::Get().Subscribe<MouseButtonPressedEvent>(BIND_FN(OnMouseButtonPressed), this);
        EventBroker::Get().Subscribe<KeyboardPressEvent>(BIND_FN(OnKeyEvent), this);
    };

    void EditorLayer::OnDetach() {
        std::cout << "EditorLayer detached" << std::endl;
        EventBroker::Get().UnsubscribeAll(this);
    };

    void EditorLayer::OnUpdate(TimeStep ts)
    {
        PROFILE_FUNCTION();

        m_EditorCamera->OnUpdate(ts);
        m_RenderGraph.Execute(ts);
    };

    void EditorLayer::OnImGuiRender()
    {
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        DrawMenuBar();
        DrawStats();
        DrawViewport();
        DrawToolbar();

        m_SceneHierarchyPanel.OnImGuiRender();
        m_ContentBrowserPanel.OnImGuiRender();
        m_SettingsPanel.OnImGuiRender();

        Renderer::ResetStatistics();
    }

    void EditorLayer::UpdateHoveredEntity()
    {
        glm::vec2 mousePos = m_Viewport.GetMousePosition();

        int mouseX = (int)mousePos.x;
        int mouseY = (int)(m_Viewport.Size.y - mousePos.y - 1);
        float width = m_Viewport.Size.x;
        float height = m_Viewport.Size.y;

        // 1. Calculate Ray
        glm::vec3 rayOrigin = m_EditorCamera->GetPosition();
        glm::mat4 cameraViewMatrix = m_EditorCamera->GetViewMatrix();
        glm::mat4 cameraProjection = m_EditorCamera->GetProjection();
        glm::vec3 rayDir = ScreenPosToWorldRay(mouseX, mouseY, width, height, cameraViewMatrix, cameraProjection);

        // 2. Variables to track the closest hit
        entt::entity selectedEntity = entt::null;
        float closestDistance = std::numeric_limits<float>::max();

        // 3. Iterate all entities with a Mesh
        auto view = m_EditingScene->GetRegistry().view<TransformComponent, ModelComponent>();
        for (auto entity : view)
        {
            auto [transformComponent, modelComponent] = view.get<TransformComponent, ModelComponent>(entity);

            for (auto& mesh : modelComponent.model->GetMeshes())
            {
                float hitDistance;
                // Test if the ray hits this mesh's AABB
                if (RayIntersectsAABB(rayOrigin, rayDir, mesh->GetAABB(), transformComponent.GetTransform(), hitDistance))
                {
                    // If it hits, check if it's the CLOSEST object we've hit so far
                    // (prevents clicking on a wall behind a tank and selecting the wall)
                    if (hitDistance < closestDistance)
                    {
                        closestDistance = hitDistance;
                        selectedEntity = entity;
                    }
                }
            }
        }

        // 4. Handle Selection!
        if (selectedEntity != entt::null)
        {
            Entity entity = {selectedEntity, m_EditingScene.get()};
            m_SceneHierarchyPanel.SetSelectedEntity(entity);
        }
        else {
            Entity entity;
            m_SceneHierarchyPanel.SetSelectedEntity(entity);
        }
    };

    void EditorLayer::UpdateSelectedEntity()
    {
        Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (!selectedEntity || !selectedEntity.HasComponent<TransformComponent>())
            return;

        SetupGizmo();
        UseGizmo(selectedEntity);
    };

    void EditorLayer::SetupGizmo()
    {
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        const auto& min = m_Viewport.Min;
        const auto& max = m_Viewport.Max;

        ImGuizmo::SetRect(min.x, min.y, max.x - min.x, max.y - min.y);
    };

    void EditorLayer::UseGizmo(Entity entity)
    {
        auto& tc = entity.GetComponent<TransformComponent>();
        glm::mat4 transform = tc.GetTransform();

        const glm::mat4 cameraProjection = m_EditorCamera->GetProjection();
        const glm::mat4 cameraView = m_EditorCamera->GetViewMatrix();

        bool snap = Input::IsKeyPressed(KEY_LEFT_SHIFT);
        float snapValue = 0.5f;
        if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
            snapValue = 45.0f;

        float snapValues[3] = { snapValue, snapValue, snapValue };

        ImGuizmo::Manipulate(
            glm::value_ptr(cameraView),
            glm::value_ptr(cameraProjection),
            (ImGuizmo::OPERATION)m_GizmoType,
            ImGuizmo::LOCAL,
            glm::value_ptr(transform),
            nullptr,
            snap ? snapValues : nullptr
        );

        if (ImGuizmo::IsUsing())
            tc.ApplyMatrix(transform);
    };

    void EditorLayer::DrawViewport()
    {
        ImGuiWindowFlags window_flags = 0;
        if (Input::IsKeyPressed(KEY_LEFT_CONTROL))
            window_flags |= ImGuiWindowFlags_NoMove;

        ScopedImGuiStyle noPadding(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
        ImGui::Begin("Viewport", nullptr, window_flags);

        m_Viewport.UpdateViewport();

        if (m_Viewport.Updated)
        {
            m_RenderGraph.Resize((unsigned int)m_Viewport.Size.x, (unsigned int)m_Viewport.Size.y);
            m_EditorCamera->SetViewportSize(m_Viewport.Size.x, m_Viewport.Size.y);
            m_EditingScene->OnViewportResize((unsigned int)m_Viewport.Size.x, (unsigned int)m_Viewport.Size.y);
        }

        unsigned int texID = m_RenderGraph.GetPassColorAttachmentRendererID("ScenePass", 0);
        if (texID != 0)
        {
            ImGui::Image((void*)(intptr_t)texID, ImVec2{m_Viewport.Size.x, m_Viewport.Size.y}, ImVec2{0, 1}, ImVec2{1, 0});
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE"))
            {
                const char* path = (char*)payload->Data;
                std::cout << path << std::endl;
                OpenScene(path);
            }
            ImGui::EndDragDropTarget();
        }

        if (m_SceneState == SceneState::Edit)
            UpdateSelectedEntity();
        
        ImGui::End();
    };
    
    void EditorLayer::DrawToolbar()
    {
        ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        const char* txt = m_SceneState == SceneState::Edit ? "Play" : "Stop";
        if (ImGui::Button(txt))
        {
            if (m_SceneState == SceneState::Edit)
                OnScenePlay();
            else if (m_SceneState == SceneState::Play)
                OnSceneStop();
        }

        ImGui::End();
        
    }

    void EditorLayer::DrawMenuBar()
    {
        ImGui::BeginMainMenuBar();
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save as.."))
            {
                SaveScene();
            }
            if (ImGui::MenuItem("Open.."))
            {
                OpenScene();
            }
            if (ImGui::MenuItem("Exit"))
            {
                Application::Get().Close();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    };

    void EditorLayer::DrawStats()
    {
        float current = (float) glfwGetTime();
        float delta = current - m_Previous;
        m_Previous = current;
        ImGui::Begin("Stats");
        unsigned int DrawCalls = Renderer::GetStatistics().DrawCalls;
        unsigned int VerticeCount = Renderer::GetStatistics().VerticeCount;
        unsigned int IndicesCount = Renderer::GetStatistics().IndicesCount;
        ImGui::Text("FPS count: %f", 1.f / delta);
        ImGui::Text("Drawcalls: %d", DrawCalls);
        ImGui::Text("Vertices count: %d", VerticeCount);
        ImGui::Text("Indices count: %d", IndicesCount);
        ImGui::End();
    };

    bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent &e)
    {
        if (e.GetMouseButton() == MOUSE_BUTTON_LEFT)
            UpdateHoveredEntity();

        return false;
    };

    bool EditorLayer::OnKeyEvent(KeyboardPressEvent &e)
    {
        switch (e.getKeyCode())
        {
            case KEY_W:
                m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                break;
            case KEY_R:
                m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                break;
            case KEY_D:
                m_GizmoType = ImGuizmo::OPERATION::SCALE;
                break;
            default:
                m_GizmoType = -1;
        }
        return false;
    };

    void EditorLayer::OpenScene()
    {
        std::string filepath = FileDialogs::OpenFile("*.pmblend");
        if (!filepath.empty())
        {
            m_EditingScene = MakeShared<Scene>();
            m_EditingScene->OnViewportResize(m_Viewport.Size.x, m_Viewport.Size.y);
            m_SceneHierarchyPanel.SetContext(m_EditingScene);
            SceneSerializer serializer(m_EditingScene);
            serializer.Deserialize(filepath);

            m_SceneRenderer->SetScene(m_EditingScene.get());
        }
    };

    void EditorLayer::OpenScene(const char* path)
    {
        if (path != nullptr)
        {
            m_EditingScene = std::static_pointer_cast<Scene>(AssetManager::LoadAsset(path));
            m_EditingScene->OnViewportResize(m_Viewport.Size.x, m_Viewport.Size.y);
            m_SceneHierarchyPanel.SetContext(m_EditingScene);

            m_SceneRenderer->SetScene(m_EditingScene.get());
        }
    }

    void EditorLayer::SaveScene()
    {
        std::string filepath = FileDialogs::SaveFile("*.pmblend");
        if (!filepath.empty())
        {
            SceneSerializer serializer(m_EditingScene);
            serializer.Serialize(filepath);
        }
    };

    void EditorLayer::OnScenePlay()
    {
        m_SceneState = SceneState::Play;
        m_RuntimeScene = Scene::Copy(m_EditingScene);
        m_SceneRenderer->SetScene(m_RuntimeScene.get());
        m_RuntimeScene->OnRuntimeStart();
    }

    void EditorLayer::OnSceneStop()
    {
        m_SceneState = SceneState::Edit;
        m_RuntimeScene->OnRuntimeStop();
        m_SceneRenderer->SetScene(m_EditingScene.get());
        m_RuntimeScene = nullptr;
    }

};
