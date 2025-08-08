#include "../pch.h"
#include "../include/ImGuiLayer.h"
#include "../include/Application.h"
#include "../include/Renderer.h"

ImGuiLayer::~ImGuiLayer()
{
};

void ImGuiLayer::OnAttach()
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();

    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
    m_Previous = glfwGetTime();
    std::cout << "Imguilayer attached" << std::endl;
};

void ImGuiLayer::OnDetach()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    std::cout << "Imguilayer detached" << std::endl;
};

void ImGuiLayer::OnImGuiRender() {
    // static bool show = true;
    // ImGui::ShowDemoWindow(&show);
    float current = (float) glfwGetTime();
    float delta = current - m_Previous;
    m_Previous = current;

    // Enable docking
    
    ImGuiIO& io = ImGui::GetIO();
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    ImGui::Begin("Some shit");
    if (ImGui::BeginMenu("Options"))
    {
        // Disabling fullscreen would allow the window to be moved to the front of other windows,
        // which we can't undo at the moment without finer window depth/z control.
        ImGui::MenuItem("Fullscreen", NULL);
        ImGui::MenuItem("Padding", NULL);
        ImGui::Separator();
        ImGui::Separator();
        ImGui::EndMenu();
    }

    unsigned int DrawCalls = Renderer::GetStatistics().DrawCalls;
    unsigned int VerticeCount = Renderer::GetStatistics().VerticeCount;
    unsigned int IndicesCount = Renderer::GetStatistics().IndicesCount;
    ImGui::Text("FPS count: %f", 1.f / delta);
    ImGui::Text("Drawcalls: %d", DrawCalls);
    ImGui::Text("Vertices count: %d", VerticeCount);
    ImGui::Text("Indices count: %d", IndicesCount);
    ImGui::End();
    Renderer::ResetStatistics();
};

void ImGuiLayer::Begin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
};

void ImGuiLayer::End()
{
    ImGuiIO& io = ImGui::GetIO();
    Application& app = Application::Get();
    io.DisplaySize = ImVec2(app.GetWindow().getWidth(), app.GetWindow().getHeight());

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup);
    }
};
