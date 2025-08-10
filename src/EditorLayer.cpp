#include "../pch.h"
#include "../include/MouseButtonCodes.h"
#include "../include/EditorLayer.h"
#include "../include/Renderer.h"
#include "../include/Input.h"
#include "../include/KeyCodes.h"
#include "../include/Raytracing.h"
#include "../include/AssimpLoader.h"
#include "../include/Application.h"

EditorLayer::EditorLayer() : m_CameraController(std::make_shared<Camera>(), 2.0f)
{
    AssimpLoader loader;
    m_Model = loader.Load("/home/tom/Coding/c++/opengl/assets/Leopard2A4.assbin");
    FramebufferSpecification spec;
    spec.width = 500;
    spec.height = 500;
    m_Framebuffer = Framebuffer::Create(spec);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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

    m_Framebuffer->Bind();

    RenderCommand::SetClearColor({0.2f, 0.2f, 0.2f, 1.0f});
    RenderCommand::Clear();

    if (m_SelectedMesh && Input::IsKeyPressed(KEY_G))
    {
        m_SelectedMesh->position += glm::vec3(0.0f, 0.0f, 1.0f); // Example movement
    }
    if (m_SelectedMesh && Input::IsKeyPressed(KEY_H))
    {
        m_SelectedMesh->position += glm::vec3(0.0f, 0.0f, -1.0f); // Example movement
    }

    if (m_ViewportFocused)
    {
        m_CameraController.OnUpdate(ts);
        Renderer::BeginScene(m_CameraController.GetCamera());
    }

    std::vector<std::shared_ptr<Mesh>> meshes = m_Model->GetMeshes();

    for (auto &mesh : meshes)
    {
        Renderer::SetTexture(mesh);
        Renderer::Submit(mesh);
    }

    Renderer::EndScene();
    m_Framebuffer->Unbind();
};

void EditorLayer::OnImGuiRender()
{
    // Enable docking
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    DrawStats();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Viewport");
    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();
    Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    if (viewportSize.x != m_ViewportSize.x || viewportSize.y != m_ViewportSize.y)
    {
        m_ViewportSize = {viewportSize.x, viewportSize.y};
        m_Framebuffer->Resize(m_ViewportSize.x, m_ViewportSize.y);

        m_CameraController.Resize(m_ViewportSize.x, m_ViewportSize.y);
    }
    unsigned int viewportTexture = m_Framebuffer->GetColorAttachmentRendererID();
    ImGui::Image((void*)viewportTexture, ImVec2{m_ViewportSize.x, m_ViewportSize.y}, ImVec2{0, 1}, ImVec2{1, 0});
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
    m_CameraController.OnEvent(e);
};

bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent &e)
{
    if (e.GetMouseButton() == MOUSE_BUTTON_LEFT)
    {
        auto [x, y] = Input::GetMousePosition();
        std::cout << "Mouse button pressed" << std::endl;
        std::cout << x << std::endl;
        std::cout << y << std::endl;

        Raytracing::TrySelectMesh(x , y, 1919, 2137, m_CameraController.GetCamera(), m_Model, m_SelectedMesh);
    }

    return false;
};

