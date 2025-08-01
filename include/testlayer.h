#pragma once

#include "CameraController.h"
#include "Layer.h"
#include "Texture.h"
#include "TimeStep.h"
#include "VertexArray.h"
#include "Shader.h"

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

    private:
    ShaderLibrary m_ShaderLibrary;
    std::shared_ptr<VertexArray> m_VertexArray;
    CameraController m_CameraController;
    glm::vec3 m_CameraPosition;
    float m_CameraSpeed = 0.1f;

    std::shared_ptr<Texture2D> m_Texture;
};
