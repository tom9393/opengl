#include "../include/CameraController.h"
#include "../include/KeyCodes.h"
#include "../include/Input.h"
#include "../pch.h"

namespace Engine
{

    CameraController::CameraController(float aspectRatio) : m_AspectRatio(aspectRatio)
    {

        m_Position = {0.0f, 0.0f, -3.0f};
        m_WorldUp = {0.0f, 1.0f, 0.0f};
        
        m_Yaw = -90.0f;
        m_Pitch = 0.0f;
        m_MouseSensitivity = 0.1f;

        UpdateCameraVectors();
        RecalculateMatrix();

        m_ProjectionMatrix = glm::perspective(glm::radians(90.0f), 2.0f, 0.1f, 1000.0f);
    };

    void CameraController::OnUpdate(TimeStep ts)
    {
        if (Input::IsKeyPressed(KEY_W)) ProcessMovement(FORWARD, ts);
        if (Input::IsKeyPressed(KEY_S)) ProcessMovement(BACKWARD, ts);
        if (Input::IsKeyPressed(KEY_A)) ProcessMovement(LEFT, ts);
        if (Input::IsKeyPressed(KEY_D)) ProcessMovement(RIGHT, ts);
        if (Input::IsKeyPressed(KEY_SPACE)) ProcessMovement(UP, ts);
        if (Input::IsKeyPressed(KEY_LEFT_CONTROL)) ProcessMovement(DOWN, ts);

        auto [x, y] = Input::GetMousePosition();
        OnMouseMoved(x, y);

        SetPosition(m_CameraPosition);
    };

    void CameraController::ProcessMovement(Direction direction, TimeStep ts) {

        float velocity = m_CameraTranslationSpeed * ts;

        if (direction == FORWARD) m_CameraPosition += GetForward() * velocity;
        if (direction == BACKWARD) m_CameraPosition -= GetForward() * velocity;
        if (direction == LEFT) m_CameraPosition -= GetRight() * velocity;
        if (direction == RIGHT) m_CameraPosition += GetRight() * velocity;
        if (direction == UP) m_CameraPosition += GetUp() * velocity;
        if (direction == DOWN) m_CameraPosition -= GetUp() * velocity;
    }

    void CameraController::SetProjection(float fov, float aspectRatio, float near, float far)
    {
        m_ProjectionMatrix = glm::perspective(fov, aspectRatio, near, far);
        RecalculateMatrix();
    };

    void CameraController::OnMouseMoved(float xpos, float ypos)
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


    void CameraController::UpdateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        front.y = sin(glm::radians(m_Pitch));
        front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        m_Forward = glm::normalize(front);
        m_Right = glm::normalize(glm::cross(m_Forward, m_WorldUp));
        m_Up = glm::normalize(glm::cross(m_Right, m_Forward));
    };

    void CameraController::RecalculateMatrix()
    {
        m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Forward, m_WorldUp);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    };

    void CameraController::SetFirstMouse(bool mouseStatus)
    {
        m_FirstMouse = mouseStatus;
    };

};
