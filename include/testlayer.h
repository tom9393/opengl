#pragma once

#include "CameraController.h"
#include "Layer.h"
#include "Ray.h"
#include "AABB.h"
#include "Texture.h"
#include "TimeStep.h"
#include "Shader.h"
#include "events/MouseEvent.h"
#include "model.h"

class Testlayer : public Layer
{
    public:
    Testlayer();
    ~Testlayer();

    void OnAttach();
    void OnDetach();
    void OnUpdate(TimeStep ts);
    void OnImGuiRender();
    void OnEvent(Event& event);
    bool OnMouseButtonPressed(MouseButtonPressedEvent &e);

    Ray CastRayFromMouse(float x, float y, float width, float height, std::shared_ptr<Camera> camera);
    bool RayIntersectsAABB(const Ray& ray, const AABB& box, float& t);
    void TrySelectMesh(float mouseX, float mouseY, float screenWidth, float screenHeight, std::shared_ptr<Camera> camera, std::shared_ptr<Model> model);

    private:
    std::shared_ptr<Model> m_Model;
    CameraController m_CameraController;
    bool m_CameraActive = true;
    std::vector<glm::vec3> m_Indices;
    float m_Previous = 0;

    std::shared_ptr<Texture2D> m_Texture;
    std::shared_ptr<Mesh> m_SelectedMesh;
};
