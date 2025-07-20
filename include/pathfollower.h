#ifndef PATHFOLLOWER
#define PATHFOLLOWER
#include "path.h"
#include "transform.h"

class PathFollower {
public:
    PathFollower(Transformable* target, Path* path, float speed, float turnRateRad) : target(target), path(path), speed(speed), turnRate(turnRateRad) {}

    void update(float deltaTime) {
        if (!target || !path) return;

        glm::vec3 currentPos = target->getPosition();
        glm::vec3 targetPoint = path->getCurrentTarget();

        // Check if we've reached the current node
        float dist = glm::distance(currentPos, targetPoint);
        if (dist < thresholdDistance) {
            path->advanceIfClose(currentPos, thresholdDistance);
            targetPoint = path->getCurrentTarget();
        }

        // Direction and rotation update
        glm::vec3 forward = Transform::getForwardDirection(*target);
        glm::vec3 desiredDir = glm::normalize(targetPoint - currentPos);

        float dot = glm::clamp(glm::dot(forward, desiredDir), -1.0f, 1.0f);
        float angle = std::acos(dot);

        // Only rotate if not already directly facing
        if (angle > 0.001f) {
            glm::vec3 axis = glm::normalize(glm::cross(forward, desiredDir));
            if (glm::length(axis) > 0.001f) {
                float actualTurn = glm::min(turnRate * deltaTime, angle);
                glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), actualTurn, axis);
                glm::vec3 newForward = glm::normalize(glm::vec3(rotMat * glm::vec4(forward, 0.0f)));

                applyForwardAsRotation(*target, newForward);
            }
        }

        // Move forward
        glm::vec3 velocity = forward * speed;
        target->setPosition(currentPos + velocity * deltaTime);
    }

    void setThreshold(float d) { thresholdDistance = d; }

private:
    Transformable* target;
    Path* path;
    float speed;
    float turnRate;
    float thresholdDistance = 0.5f;

    void applyForwardAsRotation(Transformable& transformable, const glm::vec3& forward) {
        glm::vec3 dir = glm::normalize(forward);

        float yaw = std::atan2(dir.x, dir.z);
        float pitch = std::atan2(-dir.y, std::sqrt(dir.x * dir.x + dir.z * dir.z));

        glm::vec3 currentRot = transformable.getRotation();
        glm::vec3 targetRot(pitch, yaw, 0.0f);
        transformable.setRotation(glm::mix(currentRot, targetRot, 0.15f)); // Smooth
    }
};



#endif
