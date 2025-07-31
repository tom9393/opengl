#ifndef CAMERA_H
#define CAMERA_H

#include "../pch.h"

#include "Transformable.h"
#include "Transform.h"

const float SPEED = 10.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;
const glm::mat4 PROJECTION = glm::perspective(glm::radians(60.0f), (float)1600 / (float)1600, 0.1f, 100.0f);

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera : public Transformable
{
    public:
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    glm::mat4 projection;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    Camera(glm::vec3 position = glm::vec3(-12.0f, 7.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), glm::mat4 projection = PROJECTION) : Transformable(position), Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM), WorldUp(up), projection(projection)
    {
        updateCameraVectors();
        Transform::rotateTowards(*this, glm::vec3(0.0f));
    }

    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch);
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseScroll(float yoffset);
    glm::mat4 getViewMatrix();
    glm::mat4 getProjection();

    private:
    void updateCameraVectors();
};

#endif
