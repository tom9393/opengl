#include "../pch.h"
#include "../include/ImGuiLayer.h"
#include "../include/Application.h"

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

    m_BlockEvents = true;

    GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
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
};

void ImGuiLayer::OnEvent(Event &e)
{
    if (m_BlockEvents)
    {
        ImGuiIO &io = ImGui::GetIO();
        e.handled |= e.isInCategory(EventCategoryMouse) & io.WantCaptureMouse;
        e.handled |= e.isInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
    }
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
