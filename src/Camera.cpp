#include "../include/Camera.h"

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {

    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;
    Transform::rotateY(*this, -xoffset / 50);
    Transform::rotateX(*this, -yoffset / 50);

    float pitch = getRotation().x;

    if (constrainPitch) {
        if (pitch > 1.5f)
            Transform::rotateX(*this, yoffset / 50);
        if (pitch < -1.5f)
            Transform::rotateX(*this, yoffset / 50);
    }

    updateCameraVectors();
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {

    glm::vec3 pos = getPosition();
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        pos += Front * velocity;
    if (direction == BACKWARD)
        pos -= Front * velocity;
    if (direction == LEFT)
        pos -= Right * velocity;
    if (direction == RIGHT)
        pos += Right * velocity;
    if (direction == UP)
        pos += Up * velocity;
    if (direction == DOWN)
        pos -= Up * velocity;
    setPosition(pos);
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(getPosition(), getPosition() + Front, Up);
}

glm::mat4 Camera::getProjection() {
    return projection;
}

void Camera::ProcessMouseScroll(float yoffset) {};

void Camera::updateCameraVectors() {

    Front = glm::normalize(Transform::getForwardDirection(*this));
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}
