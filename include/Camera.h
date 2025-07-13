#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
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

class IView
{
    public:
    virtual glm::mat4 getViewMatrix() = 0;
    virtual glm::mat4 getProjection() = 0;
    virtual glm::vec3 getPosition() = 0;

    virtual ~IView() = default;
};

class Camera : public IView
{
    public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    glm::mat4 projection;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH, glm::mat4 projection = PROJECTION) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM), Position(position), WorldUp(up), Yaw(yaw), Pitch(pitch), projection(projection)
    {
        updateCameraVectors();
    }

    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch);
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseScroll(float yoffset);
    glm::mat4 getViewMatrix();
    glm::mat4 getProjection();
    glm::vec3 getPosition();

    private:
    void updateCameraVectors();
};

#endif
