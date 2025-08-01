#pragma once

#include "../pch.h"

enum class CameraType
{
    Orthographic,
    Perspective
};

class Camera
{
    public:

    glm::vec3 &GetPosition() { return m_Position; }
    void SetPosition(glm::vec3 &pos) { m_Position = pos; RecalculateMatrix(); }

    glm::mat4 &GetProjectionMatrix() { return m_ProjectionMatrix; }
    glm::mat4 &GetViewMatrix() { return m_ViewMatrix; }
    glm::mat4 &GetViewProjectionMatrix() { return m_ViewProjectionMatrix; }

    protected:
    virtual void RecalculateMatrix() = 0;

    protected:
    glm::vec3 m_Position = {0.0f, 0.0f, 0.0f};
    float m_Rotation = 0.0f;
    glm::mat4 m_ProjectionMatrix = {1.0f};
    glm::mat4 m_ViewMatrix = {1.0f};
    glm::mat4 m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
};

class OrthographicCamera : public Camera
{
    public:
    OrthographicCamera();

    void SetProjection(float aspectRatio, float zoomLevel);

    private:
    void RecalculateMatrix() override;
};

class PerspectiveCamera : public Camera
{
    public:
    PerspectiveCamera();

    void SetProjection(float aspectRatio, float fov, float near, float far);

    private:
    void RecalculateMatrix() override;

    private:
    glm::vec3 m_Up = {0.0f, 1.0f, 0.0f};
    glm::vec3 m_Front = {0.0f, 0.0f, -1.0f};
};
