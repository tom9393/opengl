#pragma once

#include "TimeStep.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Camera.h"

const float SPEED = 10.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

enum Direction {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class CameraController
{
    public:

    CameraController(std::shared_ptr<Camera> camera, float aspectRatio) : m_AspectRatio(aspectRatio), m_Camera(camera) {}

    void OnEvent(Event &e);
    void OnUpdate(TimeStep ts);
    void Resize(float width, float height);

    std::shared_ptr<Camera> &GetCamera() { return m_Camera; }
    const std::shared_ptr<Camera> &GetCamera() const { return m_Camera; }

    private:
    bool OnWindowResize(WindowResizeEvent &e);
    void ProcessMovement(Direction direction, TimeStep ts);

    private:
    glm::vec3 m_CameraPosition = {0.0f, 0.0f, 0.0f};
    float m_CameraRotation = 0.0f;
    float m_CameraTranslationSpeed = 5.0f;
    float m_CameraRotationSpeed = 2.0f;
    float m_AspectRatio;
    float m_ZoomLevel = 1.0f;
    std::shared_ptr<Camera> m_Camera;
};
