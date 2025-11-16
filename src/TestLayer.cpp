#include "../pch.h"
#include "../include/MouseButtonCodes.h"
#include "../include/TestLayer.h"
#include "../include/Render/Renderer.h"
#include "../include/Input.h"
#include "../include/AssimpLoader.h"
#include "../include/Events/EventBroker.h"

namespace Engine
{

    Testlayer::Testlayer()
    {
        AssimpLoader loader;
        m_Model = loader.Load("/home/tom/Coding/c++/opengl/assets/Leopard2A4.assbin", 0);
    };

    Testlayer::~Testlayer()
    {
    };

    void Testlayer::OnAttach() {
        std::cout << "Testlayer attached" << std::endl;
        EventBroker::Get().Subscribe<MouseButtonPressedEvent>(BIND_FN(OnMouseButtonPressed), this);
    };

    void Testlayer::OnDetach() {
        std::cout << "Testlayer detached" << std::endl;
    };

    void Testlayer::OnUpdate(TimeStep ts)
    {

        PROFILE_FUNCTION();

        RenderCommand::SetClearColor({0.2f, 0.2f, 0.2f, 1.0f});
        RenderCommand::Clear();

        if (m_CameraActive)
        {
            //m_CameraController.OnUpdate(ts);
            // Renderer::BeginScene(m_CameraController.GetCamera());
        }

        std::vector<std::shared_ptr<Mesh>> meshes = m_Model->GetMeshes();

        for (auto &mesh : meshes)
        {
            Renderer::SetTexture(mesh->GetTextureLayout());
             glm::mat4 a = {1.0f};
            Renderer::Submit(mesh, a);
        }

        Renderer::EndScene();
    };

    void Testlayer::OnImGuiRender()
    {
        // static bool show = true;
        // ImGui::ShowDemoWindow(&show);
        float current = (float) glfwGetTime();
        float delta = current - m_Previous;
        m_Previous = current;

        // Enable docking
        
        ImGui::Begin("Stats");
        unsigned int DrawCalls = Renderer::GetStatistics().DrawCalls;
        unsigned int VerticeCount = Renderer::GetStatistics().VerticeCount;
        unsigned int IndicesCount = Renderer::GetStatistics().IndicesCount;
        ImGui::Text("FPS count: %f", 1.f / delta);
        ImGui::Text("Drawcalls: %d", DrawCalls);
        ImGui::Text("Vertices count: %d", VerticeCount);
        ImGui::Text("Indices count: %d", IndicesCount);

        ImGui::End();
        Renderer::ResetStatistics();
    }

    void Testlayer::OnEvent(Event& e)
    {
        return;
    };

    bool Testlayer::OnMouseButtonPressed(MouseButtonPressedEvent &e)
    {
        if (e.GetMouseButton() == MOUSE_BUTTON_LEFT)
        {
            auto [x, y] = Input::GetMousePosition();
            std::cout << "Mouse button pressed" << std::endl;
            std::cout << x << std::endl;
            std::cout << y << std::endl;
        }

        if (e.GetMouseButton() == MOUSE_BUTTON_RIGHT)
        {
            m_CameraActive = !m_CameraActive;
        }

        return false;
    };

};
