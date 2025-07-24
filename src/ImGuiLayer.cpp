#include "../pch.h"
#include <GLFW/glfw3.h>
#import "../include/ImGuiLayer.h"
#import "../include/Application.h"

ImGuiLayer::~ImGuiLayer()
{
};

void ImGuiLayer::OnAttach() {

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();

    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

};

void ImGuiLayer::OnDetach() {
    std::cout << "Detatch" << std::endl;
};

void ImGuiLayer::OnUpdate() {
    std::cout << "Update" << std::endl;
    ImGuiIO& io = ImGui::GetIO();
    Application& app = Application::Get();
    io.DisplaySize = ImVec2(app.GetWindow().getWidth(), app.GetWindow().getHeight());

    float time = (float)glfwGetTime();
    io.DeltaTime = m_Time > 0.0f ? (time - m_Time) : (1.0f / 60.0f);
    m_Time = time;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    static bool show = true;
    ImGui::ShowDemoWindow();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
};

void ImGuiLayer::OnImGuiRender() {
    std::cout << "On render" << std::endl;
};

void ImGuiLayer::OnEvent(Event& event) {
    std::cout << event.getName() << std::endl;
    std::cout << "In the Imgui layer" << std::endl;
};
