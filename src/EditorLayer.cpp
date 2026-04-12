#include "../pch.h"
#include "Asset/AssetManager.h"
#include "Core/MouseButtonCodes.h"
#include "EditorLayer.h"
#include "Render/Renderer.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Core/Application.h"
#include "Scene/Entity.h"
#include "Scene/SceneSerializer.h"
#include "Utils/PlatformUtils.h"
#include "Utils/ScopedImGuiStyle.h"
#include "Events/EventBroker.h"
#include "Scripting/ScriptEngine.h"
#include "Utils/GizmoManager.h"
#include "Asset/PrimitiveBuilder.h"

namespace Engine
{

    EditorLayer::EditorLayer()
    {
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        io.Fonts->AddFontFromFileTTF("../assets/fonts/opensans/OpenSans-Bold.ttf", 18.0f);
        io.FontDefault = io.Fonts->AddFontFromFileTTF("../assets/fonts/opensans/OpenSans-Regular.ttf", 18.0f);

        m_EditorCamera = MakeShared<EditorCamera>(90.0f, 2.0f, 0.1f, 1000.0f);
        m_EditingScene = MakeShared<Scene>();
        m_SceneRenderer = MakeShared<SceneRenderer>(m_EditingScene.get());
        m_SceneRenderer->OnViewportResize(500, 500);
        m_SceneHierarchyPanel.SetContext(m_EditingScene);
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

        switch (m_SceneState) {
            case Engine::SceneState::Edit:
            {
                m_EditorCamera->OnUpdate(ts);
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
        if (!m_Viewport.Hovered) return;
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
        auto view = m_EditingScene->GetRegistry().view<TransformComponent, MeshComponent>();
        for (auto& e : view)
        {
            auto [transformComponent, meshComponent] = view.get<TransformComponent, MeshComponent>(e);

            Shared<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshComponent.MeshHandle);

            float hitDistance;
            // Test if the ray hits this mesh's AABB
            if (RayIntersectsAABB(rayOrigin, rayDir, mesh->GetAABB(), transformComponent.GetTransform(), hitDistance))
            {
                // If it hits, check if it's the CLOSEST object we've hit so far
                // (prevents clicking on a wall behind a tank and selecting the wall)
                if (hitDistance < closestDistance)
                {
                    closestDistance = hitDistance;
                    selectedEntity = e;
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

        auto& tc = selectedEntity.GetComponent<TransformComponent>();
        glm::mat4 transform = tc.GetTransform();

        // 1. Prepare
        GizmoManager::BeginFrame();
        GizmoManager::SetViewport(m_Viewport.Min, m_Viewport.Max - m_Viewport.Min);

        // 2. Execute
        bool snap = Input::IsKeyPressed(KEY_LEFT_SHIFT);
        auto result = GizmoManager::Manipulate(*m_EditorCamera, transform, m_GizmoType, snap);

        // 3. Apply
        if (result.IsUsing) {
            tc.ApplyMatrix(result.Transform);
        }
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
            m_EditorCamera->SetViewportSize((unsigned int)m_Viewport.Size.x, (unsigned int)m_Viewport.Size.y);
            m_SceneRenderer->OnViewportResize((unsigned int)m_Viewport.Size.x, (unsigned int)m_Viewport.Size.y);
        }

        unsigned int texID = m_SceneRenderer->GetRenderGraph().GetPassColorAttachmentRendererID("ScenePass", 0);
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

        if (ImGui::BeginPopupContextWindow("SceneHierarchyContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            // You can add other standard options here too, like "Create Empty Entity"
            
            // Your 3D Object Menu
            if (ImGui::BeginMenu("Create 3D Object"))
            {
                if (ImGui::MenuItem("Cube"))
                {
                    Entity entity = m_EditingScene->CreateEntity("Cube");
                    UUID meshID = PrimitiveBuilder::CreateCube();
                    Shared<Material> cubeMaterial = MakeShared<Material>();
                    cubeMaterial->TargetShader = ShaderLibrary::Get("Primitive");
                    UUID materialID = AssetManager::CreateMemoryAsset(AssetType::Material, "CubeMaterial", cubeMaterial);
                    
                    // Adjust this to match your actual Component syntax
                    entity.AddComponent<MeshComponent>(meshID); 
                    entity.AddComponent<TransformComponent>(); 
                    entity.AddComponent<MaterialComponent>(materialID);
                }

                if (ImGui::MenuItem("Plane"))
                {
                    Entity entity = m_EditingScene->CreateEntity("Plane");
                    UUID meshID = PrimitiveBuilder::CreatePlane();
                    Shared<Material> planeMaterial = MakeShared<Material>();
                    planeMaterial->TargetShader = ShaderLibrary::Get("Primitive");
                    UUID materialID = AssetManager::CreateMemoryAsset(AssetType::Material, "CubeMaterial", planeMaterial);

                    entity.AddComponent<MeshComponent>(meshID);
                    entity.AddComponent<TransformComponent>(); 
                    entity.AddComponent<MaterialComponent>(materialID);
                }

                if (ImGui::MenuItem("Sphere"))
                {
                    Entity entity = m_EditingScene->CreateEntity("Sphere");
                    UUID meshID = PrimitiveBuilder::CreateSphere();
                    Shared<Material> sphereMaterial = MakeShared<Material>();
                    sphereMaterial->TargetShader = ShaderLibrary::Get("Primitive");
                    UUID materialID = AssetManager::CreateMemoryAsset(AssetType::Material, "CubeMaterial", sphereMaterial);

                    entity.AddComponent<MeshComponent>(meshID);
                    entity.AddComponent<TransformComponent>(); 
                    entity.AddComponent<MaterialComponent>(materialID);
                }

                ImGui::EndMenu();
            }

            // You MUST call EndPopup if BeginPopupContextWindow returns true
            ImGui::EndPopup();
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
        ImGui::Begin("Stats");

        unsigned int DrawCalls = Renderer::GetStatistics().DrawCalls;
        unsigned int MaterialSwitches = Renderer::GetStatistics().MaterialSwitches;
        unsigned int VerticeCount = Renderer::GetStatistics().VerticeCount;
        unsigned int IndicesCount = Renderer::GetStatistics().IndicesCount;
        float delta = Application::Get().GetWindow().GetFrameTime();

        ImGui::Text("FPS count: %f", 1.f / delta);
        ImGui::Text("Drawcalls: %d", DrawCalls);
        ImGui::Text("Material switches: %d", MaterialSwitches);
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
            case KEY_F1:
                if (m_SceneState == SceneState::Edit) OnScenePlay();
                else OnSceneStop();
                break;
            case KEY_F5:
                ScriptEngine::ReloadScripts();
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
            m_EditingScene = AssetManager::GetAsset<Scene>(AssetManager::ImportAsset(path));
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
        Application::Get().GetWindow().DisableCursor();
    }

    void EditorLayer::OnSceneStop()
    {
        m_SceneState = SceneState::Edit;

        m_RuntimeScene->OnRuntimeStop();
        m_SceneRenderer->SetScene(m_EditingScene.get());
        m_RuntimeScene = nullptr;
        Application::Get().GetWindow().EnableCursor();
    }

};
