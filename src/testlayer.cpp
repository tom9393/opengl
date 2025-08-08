#include "../pch.h"
#include "../include/MouseButtonCodes.h"
#include "../include/testlayer.h"
#include "../include/Renderer.h"
#include "../include/Input.h"
#include "../include/KeyCodes.h"
#include "../include/Raytracing.h"

Testlayer::Testlayer() : m_CameraController(std::make_shared<Camera>(), 2.0f)
{
    stbi_set_flip_vertically_on_load(true);  
    m_Model = std::make_shared<Model>("/home/tom/Coding/c++/opengl/assets/Leopard2A4.assbin");
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

    unsigned int idx = 0;
    for (auto &mesh : meshes)
    {
        std::vector<std::shared_ptr<Texture2D>> textures = mesh->GetTextures();
        // for (auto& texture : textures)
        // {
        //     Renderer::SetTexture(texture, mesh);
        //     idx++;
        // }
        if (idx == 0)
        {
            Renderer::SetTexture(textures[0], m_Model->GetTextures());
            idx++;
        }
        Renderer::Submit(mesh, glm::mat4(1.0f));
    }

    Renderer::EndScene();
};

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

