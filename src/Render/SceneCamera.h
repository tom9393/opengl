#pragma once

#include "Camera.h"

namespace Engine
{

        class SceneCamera : public Camera
        {
                public:
                enum class ProjectionType { Perspective = 0, Orthographic = 1 };

                public:
                SceneCamera();
                virtual ~SceneCamera() = default;

                void SetPerspective(float fov, float near, float far);
                void SetOrthographic(float size, float near, float far);

                void SetViewportSize(unsigned int width, unsigned int height);
                unsigned int GetViewportWidth() const { return m_ViewportWidth; }
                unsigned int GetViewportHeight() const { return m_ViewportHeight; }

                float GetPerspectiveVerticalFOV() const { return m_PerspectiveFOV; }
                void SetPerspectiveVerticalFOV(float verticalFov) { m_PerspectiveFOV = verticalFov; RecalculateProjection(); }
                float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
                void SetPerspectiveNearClip(float near) { m_PerspectiveNear = near; RecalculateProjection(); }
                float GetPerspectiveFarClip() const { return m_PerspectiveFar; }
                void SetPerspectiveFarClip(float far) { m_PerspectiveFar = far; RecalculateProjection(); }

                float GetOrthographicSize() const { return m_OrthographicSize; }
                void SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjection(); }
                float GetOrthographicNearClip() const { return m_OrthographicNear; }
                void SetOrthographicNearClip(float near) { m_OrthographicNear = near; RecalculateProjection(); }
                float GetOrthographicFarClip() const { return m_OrthographicFar; }
                void SetOrthographicFarClip(float far) { m_OrthographicFar = far; RecalculateProjection(); }

                float GetNearClip() const { return m_ProjectionType == ProjectionType::Perspective ? m_PerspectiveNear : m_OrthographicNear; }
                float GetFarClip()  const { return m_ProjectionType == ProjectionType::Perspective ? m_PerspectiveFar  : m_OrthographicFar;  }

                ProjectionType GetProjectionType() const { return m_ProjectionType; }
                void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecalculateProjection(); }

                private:
                void RecalculateProjection();

                private:
                ProjectionType m_ProjectionType = ProjectionType::Perspective;

                float m_PerspectiveFOV = glm::radians(45.0f);
                float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;

                float m_OrthographicSize = 10.0f;
                float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

                float m_AspectRatio = 1.0f;
                unsigned int m_ViewportWidth = 0, m_ViewportHeight = 0;
        };

};
