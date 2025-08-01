#include "../include/Camera.h"
#include <glm/ext/matrix_clip_space.hpp>

OrthographicCamera::OrthographicCamera()
{
    m_ProjectionMatrix = glm::ortho(-1.5f, 1.5f, -0.75f, 0.75f);
    m_Position = {0.0f, 0.0f, 0.0f};
};

void OrthographicCamera::SetProjection(float aspectRatio, float zoomLevel)
{
    m_ProjectionMatrix = glm::ortho(-aspectRatio * zoomLevel, aspectRatio * zoomLevel, -zoomLevel, zoomLevel);
    RecalculateMatrix();
};

void OrthographicCamera::RecalculateMatrix()
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) * glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    m_ViewMatrix = glm::inverse(transform);
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
};






PerspectiveCamera::PerspectiveCamera()
{
    m_ProjectionMatrix = glm::perspective(glm::radians(90.0f), 2.0f, 0.1f, 100.0f);
    m_Position = {0.0f, 0.0f, -3.0f};
};

void PerspectiveCamera::SetProjection(float aspectRatio, float fov, float near, float far)
{
    m_ProjectionMatrix = glm::perspective(fov, aspectRatio, near, far);
    RecalculateMatrix();
};

void PerspectiveCamera::RecalculateMatrix()
{
    m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
};
