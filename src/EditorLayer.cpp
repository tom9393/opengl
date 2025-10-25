#include "../pch.h"
#include "../include/MouseButtonCodes.h"
#include "../include/EditorLayer.h"
#include "../include/Renderer.h"
#include "../include/Input.h"
#include "../include/KeyCodes.h"
#include "../include/Raytracing.h"
#include "../include/AssimpLoader.h"
#include "../include/Application.h"
#include "../include/Scene/Entity.h"
#include "../include/Scene/SceneSerializer.h"
#include "../include/Utils/PlatformUtils.h"
#include "../lib/ImGuizmo.h"
#include <glm/gtc/type_ptr.hpp>
#include "../include/MathTools.h"

EditorLayer::EditorLayer()
{
    AssimpLoader loader;
    m_Model = loader.Load("/home/tom/Coding/c++/opengl/assets/sukhoi.assbin");
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
};

EditorLayer::~EditorLayer()
{
};

void EditorLayer::OnAttach() {
    std::cout << "EditorLayer attached" << std::endl;
    m_Scene = MakeShared<Scene>();
    m_Scene->OnViewportResize(500, 500);
    m_SceneHierarchyPanel.SetContext(m_Scene);
};

void EditorLayer::OnDetach() {
    std::cout << "EditorLayer detached" << std::endl;
};

void EditorLayer::OnUpdate(TimeStep ts)
{

    PROFILE_FUNCTION();

    m_Framebuffer->Bind();

    RenderCommand::SetClearColor({0.2f, 0.2f, 0.2f, 1.0f});
    RenderCommand::Clear();

    m_Framebuffer->ClearAttachment(1, -1);

    if (m_SelectedMesh && Input::IsKeyPressed(KEY_G)) m_SelectedMesh->position += glm::vec3(0.0f, 0.0f, 1.0f); // Example movement
    if (m_SelectedMesh && Input::IsKeyPressed(KEY_H)) m_SelectedMesh->position += glm::vec3(0.0f, 0.0f, -1.0f); // Example movement

    m_EditorCamera->OnUpdate(ts);
    Renderer::BeginScene(m_EditorCamera->GetViewProjection());
    m_Scene->OnUpdate(ts);

    auto [mx, my] = ImGui::GetMousePos();
    mx -= m_ViewportBounds[0].x;
    my -= m_ViewportBounds[0].y;
    glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
    my = viewportSize.y - my;
    int mouseX = (int)mx;
    int mouseY = (int)my;

    if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
    {
        int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);

        m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_Scene.get());
    };

    m_Framebuffer->Unbind();
};

void EditorLayer::OnImGuiRender()
{
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

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

    DrawStats();

    m_SceneHierarchyPanel.OnImGuiRender();

    ImGuiWindowFlags window_flags = 0;
    if (Input::IsKeyPressed(KEY_LEFT_CONTROL))
        window_flags |= ImGuiWindowFlags_NoMove;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Viewport", nullptr, window_flags);

    auto viewOffset = ImGui::GetCursorPos();

    auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
    auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
    auto viewportOffset = ImGui::GetWindowPos();
    m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
    m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();

    Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    if (viewportSize.x != m_ViewportSize.x || viewportSize.y != m_ViewportSize.y)
    {
        m_ViewportSize = {viewportSize.x, viewportSize.y};
        m_Framebuffer->Resize(m_ViewportSize.x, m_ViewportSize.y);
        m_EditorCamera->SetViewportSize(viewportSize.x, viewportSize.y);
    }
    unsigned int viewportTexture = m_Framebuffer->GetColorAttachmentRendererID(0);
    ImGui::Image(viewportTexture, ImVec2{m_ViewportSize.x, m_ViewportSize.y}, ImVec2{0, 1}, ImVec2{1, 0});

    Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
    if (selectedEntity && selectedEntity.HasComponent<TransformComponent>())
    {
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

        glm::mat4 cameraProjection = m_EditorCamera->GetProjection();
        glm::mat4 cameraView = m_EditorCamera->GetViewMatrix();

        auto& tc = selectedEntity.GetComponent<TransformComponent>();
        glm::mat4 transform = tc.GetTransform();

        ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION) m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform));

        if (ImGuizmo::IsUsing())
        {
            glm::vec3 translation, rotation, scale;
            MathTools::DecomposeMatrix(transform, translation, rotation, scale);

            tc.translation = translation;
            tc.rotation = rotation;
            tc.scale = scale;
        }
    }
    
    ImGui::End();
    ImGui::PopStyleVar();

    Renderer::ResetStatistics();
}

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
        if (m_ViewportHovered && !ImGuizmo::IsOver())
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
        m_Scene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
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
