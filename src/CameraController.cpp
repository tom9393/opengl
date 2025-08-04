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
    m_Camera->SetProjection(m_AspectRatio, glm::radians(90.0f), 0.1f, 100.0f);

    return false;
}

void CameraController::OnUpdate(TimeStep ts)
{
    if (Input::IsKeyPressed(KEY_W)) ProcessMovement(FORWARD, ts);
    if (Input::IsKeyPressed(KEY_S)) ProcessMovement(BACKWARD, ts);
    if (Input::IsKeyPressed(KEY_A)) ProcessMovement(LEFT, ts);
    if (Input::IsKeyPressed(KEY_D)) ProcessMovement(RIGHT, ts);
    if (Input::IsKeyPressed(KEY_SPACE)) ProcessMovement(UP, ts);
    if (Input::IsKeyPressed(KEY_LEFT_CONTROL)) ProcessMovement(DOWN, ts);

    auto [x, y] = Input::GetMousePosition();
    m_Camera->OnMouseMoved(x, y);

    m_Camera->SetPosition(m_CameraPosition);
};

void CameraController::ProcessMovement(Direction direction, TimeStep ts) {

    float velocity = m_CameraTranslationSpeed * ts;

    if (direction == FORWARD) m_CameraPosition += m_Camera->GetForward() * velocity;
    if (direction == BACKWARD) m_CameraPosition -= m_Camera->GetForward() * velocity;
    if (direction == LEFT) m_CameraPosition -= m_Camera->GetRight() * velocity;
    if (direction == RIGHT) m_CameraPosition += m_Camera->GetRight() * velocity;
    if (direction == UP) m_CameraPosition += m_Camera->GetUp() * velocity;
    if (direction == DOWN) m_CameraPosition -= m_Camera->GetUp() * velocity;
}
