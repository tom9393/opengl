#pragma once

#include "../pch.h"
#include "TimeStep.h"


class Camera
{
    public:

    Camera();

    glm::vec3 &GetPosition() { return m_Position; }
    void SetPosition(glm::vec3 &pos) { m_Position = pos; RecalculateMatrix(); }

    glm::mat4 &GetProjectionMatrix() { return m_ProjectionMatrix; }
    glm::mat4 &GetViewMatrix() { return m_ViewMatrix; }
    glm::mat4 &GetViewProjectionMatrix() { return m_ViewProjectionMatrix; }

    void OnMouseMoved(float xpos, float ypos);
    void SetProjection(float fov, float aspectRatio, float near, float far);

    glm::vec3 &GetForward() { return m_Forward; };
    glm::vec3 &GetUp() { return m_Up; };
    glm::vec3 &GetWorldUp() { return m_WorldUp; };
    glm::vec3 &GetRight() { return m_Right; };

    private:
    void UpdateCameraVectors();
    void RecalculateMatrix();

    private:
    glm::vec3 m_Position = {0.0f, 0.0f, 0.0f};
    float m_Rotation = 0.0f;

    glm::vec3 m_WorldUp = {0.0f, 1.0f, 0.0f};

    glm::vec3 m_Up = m_WorldUp;
    glm::vec3 m_Right = m_WorldUp;
    glm::vec3 m_Forward = {0.0f, 0.0f, -1.0f};

    float m_Yaw = -90.0f;
    float m_Pitch = 0.0f;
    float m_MouseSensitivity = 0.1f;
    bool m_FirstMouse = true;
    float m_LastX = 0.0f;
    float m_LastY = 0.0f;

    glm::mat4 m_ProjectionMatrix = {1.0f};
    glm::mat4 m_ViewMatrix = {1.0f};
    glm::mat4 m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
};

