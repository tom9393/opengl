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

EditorLayer::EditorLayer()
{
    AssimpLoader loader;
    m_Model = loader.Load("/home/tom/Coding/c++/opengl/assets/sukhoi.assbin");
    FramebufferSpecification spec;
    spec.width = 500;
    spec.height = 500;
    m_Framebuffer = Framebuffer::Create(spec);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    io.Fonts->AddFontFromFileTTF("/home/tom/Coding/c++/opengl/assets/fonts/opensans/OpenSans-Bold.ttf", 18.0f);
    io.FontDefault = io.Fonts->AddFontFromFileTTF("/home/tom/Coding/c++/opengl/assets/fonts/opensans/OpenSans-Regular.ttf", 18.0f);
};

EditorLayer::~EditorLayer()
{
};

void EditorLayer::OnAttach() {
    std::cout << "EditorLayer attached" << std::endl;
    m_Scene = MakeShared<Scene>();
    m_Scene->OnViewportResize(500, 500);

    unsigned int c = 1;
    for (unsigned int i = 0; i < c; i++)
    {
        for (unsigned int j = 0; j < c; j++)
        {
            glm::vec3 translation = {i * 4.0f, 0.0f, j * 10.0f};
            glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
            glm::vec3 size = {5.0f, 5.0f, 5.0f};
            Entity entity = m_Scene->CreateEntity("Tank model");
            entity.AddComponent<TransformComponent>(translation, rotation, size);
            entity.AddComponent<ModelComponent>(m_Model);
        }
    }

    Entity cameraEntity = m_Scene->CreateEntity("Editing camera");
    Shared<EditorCamera> cam = MakeShared<EditorCamera>(90.0f, 2.0f, 0.1f, 1000.0f);
    cameraEntity.AddComponent<EditorCameraComponent>(cam);

    Entity sceneCamera = m_Scene->CreateEntity("Scene camera");
    Shared<SceneCamera> sceneCam = MakeShared<SceneCamera>();
    sceneCam->SetProjectionType(SceneCamera::ProjectionType::Orthographic);
    sceneCamera.AddComponent<SceneCameraComponent>(sceneCam);
    glm::vec3 translation = glm::vec3(1.0f, 1.0f, 1.0f);
    sceneCamera.AddComponent<TransformComponent>(translation);

    m_SceneHierarchyPanel.SetContext(m_Scene);

    SceneSerializer serializer(m_Scene);
    serializer.Serialize("/home/tom/Coding/c++/opengl/assets/example.pmblend");
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

    if (m_SelectedMesh && Input::IsKeyPressed(KEY_G)) m_SelectedMesh->position += glm::vec3(0.0f, 0.0f, 1.0f); // Example movement
    if (m_SelectedMesh && Input::IsKeyPressed(KEY_H)) m_SelectedMesh->position += glm::vec3(0.0f, 0.0f, -1.0f); // Example movement
    //if (m_ViewportFocused) m_CameraController.OnUpdate(ts);

    m_Scene->OnUpdate(ts);

    m_Framebuffer->Unbind();
};

void EditorLayer::OnImGuiRender()
{
    // Enable docking
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    DrawStats();

    m_SceneHierarchyPanel.OnImGuiRender();

    ImGuiWindowFlags window_flags = 0;
    if (Input::IsKeyPressed(KEY_LEFT_CONTROL))
        window_flags |= ImGuiWindowFlags_NoMove;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Viewport", nullptr, window_flags);
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

        //m_CameraController.Resize(m_ViewportSize.x, m_ViewportSize.y);
    }
    unsigned int viewportTexture = m_Framebuffer->GetColorAttachmentRendererID();
    ImGui::Image(viewportTexture, ImVec2{m_ViewportSize.x, m_ViewportSize.y}, ImVec2{0, 1}, ImVec2{1, 0});
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
    // m_CameraController.OnEvent(e);
};

bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent &e)
{
    // if (e.GetMouseButton() == MOUSE_BUTTON_LEFT)
    // {
    //     auto [x, y] = Input::GetMousePosition();
    //     std::cout << "Mouse button pressed" << std::endl;
    //     std::cout << x << std::endl;
    //     std::cout << y << std::endl;
    //
    //     // Raytracing::TrySelectMesh(x , y, 1919, 2137, m_CameraController.GetCamera(), m_Model, m_SelectedMesh);
    // }

    return false;
};

