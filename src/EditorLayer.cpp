#include "../pch.h"
#include "../include/MouseButtonCodes.h"
#include "../include/EditorLayer.h"
#include "../include/Renderer.h"
#include "../include/Input.h"
#include "../include/KeyCodes.h"
#include "../include/Application.h"
#include "../include/Scene/Entity.h"
#include "../include/Scene/SceneSerializer.h"
#include "../include/Utils/PlatformUtils.h"
#include "../include/Utils/ScopedImGuiStyle.h"
#include "../include/Scene/Components.h"

namespace Engine
{

    EditorLayer::EditorLayer()
    {
        FramebufferSpecification spec;
        spec.width = 500;
        spec.height = 500;
        spec.attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
        m_Framebuffer = Framebuffer::Create(spec);

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        io.Fonts->AddFontFromFileTTF("/home/tom/Coding/c++/opengl/assets/fonts/opensans/OpenSans-Bold.ttf", 18.0f);
        io.FontDefault = io.Fonts->AddFontFromFileTTF("/home/tom/Coding/c++/opengl/assets/fonts/opensans/OpenSans-Regular.ttf", 18.0f);

        m_EditorCamera = MakeShared<EditorCamera>(90.0f, 2.0f, 0.1f, 1000.0f);
        m_Scene = MakeShared<Scene>();
        m_Scene->OnViewportResize(500, 500);
        m_SceneHierarchyPanel.SetContext(m_Scene);
    };

    EditorLayer::~EditorLayer()
    {
    };

    void EditorLayer::OnAttach() {
        std::cout << "EditorLayer attached" << std::endl;
    };

    void EditorLayer::OnDetach() {
        std::cout << "EditorLayer detached" << std::endl;
    };

    void EditorLayer::OnUpdate(TimeStep ts)
    {
        PROFILE_FUNCTION();

        m_EditorCamera->OnUpdate(ts);

        BeginViewportRender();
        RenderScene(ts);
        EndViewportRender();

        UpdateHoveredEntity();
    };

    void EditorLayer::BeginViewportRender()
    {
        m_Framebuffer->Bind();
        RenderCommand::SetClearColor({0.2f, 0.2f, 0.2f, 1.0f});
        RenderCommand::Clear();
        m_Framebuffer->ClearAttachment(1, -1);
        Renderer::BeginScene(m_EditorCamera->GetViewProjection());
    };

    void EditorLayer::RenderScene(TimeStep ts)
    {
        if (m_Scene) m_Scene->OnUpdate(ts);
    };

    void EditorLayer::EndViewportRender()
    {
        m_Framebuffer->Unbind();
    };

    void EditorLayer::OnImGuiRender()
    {
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        DrawMenuBar();
        DrawStats();
        DrawViewport();

        m_SceneHierarchyPanel.OnImGuiRender();
        m_ContentBrowserPanel.OnImGuiRender();

        Renderer::ResetStatistics();
    }

    void EditorLayer::UpdateHoveredEntity()
    {
        glm::vec2 mousePos = m_Viewport.GetMousePosition();

        if (m_Viewport.Hovered && m_Viewport.Contains(mousePos))
        {
            int pixelData = m_Framebuffer->ReadPixel(1, mousePos.x, mousePos.y);
            m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_Scene.get());
        };
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
            m_Framebuffer->Resize(m_Viewport.Size.x, m_Viewport.Size.y);
            m_EditorCamera->SetViewportSize(m_Viewport.Size.x, m_Viewport.Size.y);
        }
        ImGui::Image(m_Framebuffer->GetColorAttachmentRendererID(0), ImVec2{m_Viewport.Size.x, m_Viewport.Size.y}, ImVec2{0, 1}, ImVec2{1, 0});

        UpdateSelectedEntity();
        
        ImGui::End();
    };

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
                Application::Get().close();
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

    void EditorLayer::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<MouseButtonPressedEvent>(std::bind(&EditorLayer::OnMouseButtonPressed, this, std::placeholders::_1));
        dispatcher.dispatch<KeyboardPressEvent>(std::bind(&EditorLayer::OnKeyEvent, this, std::placeholders::_1));
    };

    bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent &e)
    {
        if (e.GetMouseButton() == MOUSE_BUTTON_LEFT)
        {
            if (m_Viewport.Hovered && !ImGuizmo::IsOver())
            {
                m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
            }
        }

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
            m_Scene = MakeShared<Scene>();
            m_Scene->OnViewportResize(m_Viewport.Size.x, m_Viewport.Size.y);
            m_SceneHierarchyPanel.SetContext(m_Scene);
            SceneSerializer serializer(m_Scene);
            serializer.Deserialize(filepath);
        }
    };

    void EditorLayer::SaveScene()
    {
        std::string filepath = FileDialogs::SaveFile("*.pmblend");
        if (!filepath.empty())
        {
            SceneSerializer serializer(m_Scene);
            serializer.Serialize(filepath);
        }
    };

};
