#include "../include/Camera.h"
#include <glm/ext/matrix_clip_space.hpp>

Camera::Camera()
{
    m_Position = {0.0f, 0.0f, -3.0f};
    m_WorldUp = {0.0f, 1.0f, 0.0f};
    
    // 🔧 Add these:
    m_Yaw = -90.0f;   // Facing down -Z (OpenGL standard view)
    m_Pitch = 0.0f;
    m_MouseSensitivity = 0.1f;

    UpdateCameraVectors(); // ← important!
    RecalculateMatrix();

    m_ProjectionMatrix = glm::perspective(glm::radians(90.0f), 2.0f, 0.1f, 100.0f);
};

void Camera::SetProjection(float aspectRatio, float fov, float near, float far)
{
    m_ProjectionMatrix = glm::perspective(fov, aspectRatio, near, far);
    RecalculateMatrix();
};

void Camera::OnMouseMoved(float xpos, float ypos)
{
    if (m_FirstMouse)
    {
        m_LastX = xpos;
        m_LastY = ypos;
        m_FirstMouse = false;
    }

    float xoffset = xpos - m_LastX;
    float yoffset = m_LastY - ypos;

    m_LastX = xpos;
    m_LastY = ypos;

    xoffset *= m_MouseSensitivity;
    yoffset *= m_MouseSensitivity;

    m_Yaw += xoffset;
    m_Pitch += yoffset;

    if (m_Pitch > 89.0f) m_Pitch = 89.0f;
    if (m_Pitch < -89.0f) m_Pitch = -89.0f;

    UpdateCameraVectors();
};


void Camera::UpdateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Forward = glm::normalize(front);
    m_Right = glm::normalize(glm::cross(m_Forward, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Forward));
};

void Camera::RecalculateMatrix()
{
    m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Forward, m_WorldUp);
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
};

void Camera::SetFirstMouse(bool mouseStatus)
{
    m_FirstMouse = mouseStatus;
};
