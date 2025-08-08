#include "../pch.h"
#include "../include/MouseButtonCodes.h"
#include "../include/testlayer.h"
#include "../include/Renderer.h"
#include "../include/Input.h"
#include "../include/KeyCodes.h"
#include "../include/Raytracing.h"
#include "../include/AssimpLoader.h"

Testlayer::Testlayer() : m_CameraController(std::make_shared<Camera>(), 2.0f)
{
    AssimpLoader loader;
    m_Model = loader.Load("/home/tom/Coding/c++/opengl/assets/Leopard2A4.assbin");
    FramebufferSpecification spec;
    spec.width = 500;
    spec.height = 500;
    m_Framebuffer = Framebuffer::Create(spec);
};

Testlayer::~Testlayer()
{
};

void Testlayer::OnAttach() {
    std::cout << "Testlayer attached" << std::endl;
};

void Testlayer::OnDetach() {
    std::cout << "Testlayer detached" << std::endl;
};

void Testlayer::OnUpdate(TimeStep ts)
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

    if (m_CameraActive)
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

void Testlayer::OnImGuiRender()
{
    ImGui::Begin("Some window");
    unsigned int txt = m_Framebuffer->GetColorAttachmentRendererID();
    ImGui::Image((void*)txt, ImVec2{500, 500});
    ImGui::End();
}

void Testlayer::OnEvent(Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.dispatch<MouseButtonPressedEvent>(std::bind(&Testlayer::OnMouseButtonPressed, this, std::placeholders::_1));
    m_CameraController.OnEvent(e);
};

bool Testlayer::OnMouseButtonPressed(MouseButtonPressedEvent &e)
{
    if (e.GetMouseButton() == MOUSE_BUTTON_LEFT)
    {
        auto [x, y] = Input::GetMousePosition();
        std::cout << "Mouse button pressed" << std::endl;
        std::cout << x << std::endl;
        std::cout << y << std::endl;

        Raytracing::TrySelectMesh(x , y, 1919, 2137, m_CameraController.GetCamera(), m_Model, m_SelectedMesh);
    }

    if (e.GetMouseButton() == MOUSE_BUTTON_RIGHT)
    {
        m_CameraActive = !m_CameraActive;
        m_CameraController.GetCamera()->SetFirstMouse(m_CameraActive);
    }

    return false;
};

