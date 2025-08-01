#include "../include/CameraController.h"
#include "../include/KeyCodes.h"
#include "../include/Input.h"
#include "../pch.h"

void CameraController::OnEvent(Event &e)
{
    EventDispatcher dispatcher(e);
    dispatcher.dispatch<WindowResizeEvent>(std::bind(&CameraController::OnWindowResize, this, std::placeholders::_1));
}

bool CameraController::OnWindowResize(WindowResizeEvent &e)
{
    m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();

    if (auto ortho = std::dynamic_pointer_cast<OrthographicCamera>(m_Camera))
    {
        ortho->SetProjection(m_AspectRatio, m_ZoomLevel);
    }
    else if (auto perspective = std::dynamic_pointer_cast<PerspectiveCamera>(m_Camera))
    {
        perspective->SetProjection(m_AspectRatio, glm::radians(90.0f), 0.1f, 100.0f);
    }

    return false;
}

void CameraController::OnUpdate(TimeStep ts)
{
    if (auto ortho = std::dynamic_pointer_cast<OrthographicCamera>(m_Camera))
    {
        if (Input::IsKeyPressed(KEY_W)) m_CameraPosition.y += m_CameraTranslationSpeed * ts;
        if (Input::IsKeyPressed(KEY_S)) m_CameraPosition.y -= m_CameraTranslationSpeed * ts;
        if (Input::IsKeyPressed(KEY_A)) m_CameraPosition.x -= m_CameraTranslationSpeed * ts;
        if (Input::IsKeyPressed(KEY_D)) m_CameraPosition.x += m_CameraTranslationSpeed * ts;

        ortho->SetProjection(m_AspectRatio, m_ZoomLevel);
    }
    else if (auto perspective = std::dynamic_pointer_cast<PerspectiveCamera>(m_Camera))
    {
        if (Input::IsKeyPressed(KEY_A)) m_CameraPosition.x -= m_CameraTranslationSpeed * ts;
        if (Input::IsKeyPressed(KEY_D)) m_CameraPosition.x += m_CameraTranslationSpeed * ts;
        if (Input::IsKeyPressed(KEY_S)) m_CameraPosition.z += m_CameraTranslationSpeed * ts;
        if (Input::IsKeyPressed(KEY_W)) m_CameraPosition.z -= m_CameraTranslationSpeed * ts;

        perspective->SetProjection(m_AspectRatio, glm::radians(90.0f), 0.1f, 100.0f);
    }

    m_Camera->SetPosition(m_CameraPosition);
};
