#pragma once

#include "CameraController.h"
#include "Layer.h"
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

    private:
    ShaderLibrary m_ShaderLibrary;
    std::shared_ptr<Model> m_Model;
    CameraController m_CameraController;
    glm::vec3 m_CameraPosition;
    float m_CameraSpeed = 0.1f;

    bool m_CameraActive = true;

    std::shared_ptr<Texture2D> m_Texture;
};
