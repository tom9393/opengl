#pragma once

#include "Layer.h"
#include "OrthographicCamera.h"
#include "VertexArray.h"
#include "shader.h"

class Testlayer : public Layer
{
    public:
    Testlayer();
    ~Testlayer();

    void OnAttach();
    void OnDetach();
    void OnUpdate();
    void OnImGuiRender();
    void OnEvent(Event& event);

    private:
    Shader* shader;
    std::shared_ptr<VertexArray> m_VertexArray;
    OrthographicCamera m_Camera;
    glm::vec3 m_CameraPosition;
    float m_CameraSpeed = 0.1f;
};
