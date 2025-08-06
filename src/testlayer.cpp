#include "../pch.h"
#include "../include/MouseButtonCodes.h"
#include "../include/testlayer.h"
#include "../include/Renderer.h"
#include "../include/Input.h"
#include "../include/KeyCodes.h"

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

    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        glm::mat4 pos = glm::translate(glm::mat4(1.0f), meshes[i]->position);
        std::vector<std::shared_ptr<Texture2D>> textures = meshes[i]->GetTextures();
        for (auto& texture : textures)
        {
            Renderer::SetTexture(texture, i);
        }
        Renderer::Submit(meshes[i]->GetVao(), pos);
    }

    Renderer::EndScene();
};

void Testlayer::OnImGuiRender()
{
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
        std::cout << "Hello" << std::endl;
        std::cout << x << std::endl;
        std::cout << y << std::endl;

        TrySelectMesh(x , y, 1919, 2137, m_CameraController.GetCamera(), m_Model);
    }

    if (e.GetMouseButton() == MOUSE_BUTTON_RIGHT)
    {
        m_CameraActive = !m_CameraActive;
        m_CameraController.GetCamera()->SetFirstMouse(m_CameraActive);
    }

    return false;
};

Ray Testlayer::CastRayFromMouse(float mouseX, float mouseY, float screenWidth, float screenHeight, std::shared_ptr<Camera> camera)
{
    // Normalized Device Coordinates
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;
    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

    glm::mat4 proj = camera->GetProjectionMatrix();
    glm::mat4 view = camera->GetViewMatrix();

    // Eye space
    glm::vec4 rayEye = glm::inverse(proj) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // World space
    glm::vec3 rayWorld = glm::vec3(glm::inverse(view) * rayEye);
    rayWorld = glm::normalize(rayWorld);

    return { camera->GetPosition(), rayWorld };
}

bool Testlayer::RayIntersectsAABB(const Ray& ray, const AABB& box, float& t)
{
    float tmin = (box.min.x - ray.origin.x) / ray.direction.x;
    float tmax = (box.max.x - ray.origin.x) / ray.direction.x;
    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (box.min.y - ray.origin.y) / ray.direction.y;
    float tymax = (box.max.y - ray.origin.y) / ray.direction.y;
    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    tmin = std::max(tmin, tymin);
    tmax = std::min(tmax, tymax);

    float tzmin = (box.min.z - ray.origin.z) / ray.direction.z;
    float tzmax = (box.max.z - ray.origin.z) / ray.direction.z;
    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    t = tmin;
    return true;
}

void Testlayer::TrySelectMesh(float mouseX, float mouseY, float screenWidth, float screenHeight, std::shared_ptr<Camera> camera, std::shared_ptr<Model> model)
{
    Ray ray = CastRayFromMouse(mouseX, mouseY, screenWidth, screenHeight, camera);

    float closestT = std::numeric_limits<float>::max();
    m_SelectedMesh = nullptr;

    for (auto& mesh : model->GetMeshes())
    {
        AABB aabb = mesh->GetTransformedAABB();
        float t;
        if (RayIntersectsAABB(ray, aabb, t))
        {
            if (t < closestT)
            {
                closestT = t;
                m_SelectedMesh = mesh;
            }
        }
    }
}
