#ifndef PATH
#define PATH
#include "transform.h"
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <vector>
#include <cmath>


class Path {
public:
    Path(Transformable *transformable, float moveSpeed = 1.0f, float turnSpeed = 1.0f) : transformable(transformable), speed(moveSpeed), rotationSpeed(turnSpeed), currentTarget(0), arrivalThreshold(0.1f), loopCount(0) {}

    void setPath(const std::vector<glm::vec3>& points) {
        waypoints = points;
        currentTarget = 0;
        loopCount = 0;
    }

    void update(float deltaTime, int(*cb)(int)) {
        if (waypoints.empty()) return;

        glm::vec3 target = waypoints[currentTarget];
        glm::vec3 direction = target - transformable->getPosition();
        float distance = glm::length(direction);

        // Check if we've arrived at the current waypoint
        if (distance < arrivalThreshold) {
            currentTarget++;
            if (currentTarget >= waypoints.size()) {
                currentTarget = 0;
                loopCount++;
                cb(loopCount);
            }
            return;
        }

        // Normalize direction
        glm::vec3 dirNorm = glm::normalize(direction);

        // Move forward
        Transform::move(*transformable, dirNorm * speed * deltaTime);

        // Compute full Euler angles: yaw (Y), pitch (X)
        float yaw = atan2(dirNorm.x, dirNorm.z);
        float pitch = -asin(dirNorm.y); // assuming y-up
        float roll = 0.0f; // flat ground; could be extended to lean into curves if needed

        // Smooth interpolation
        glm::vec3 newRot = glm::vec3(
            interpolateAngle(transformable->getRotation().x, pitch, rotationSpeed * deltaTime),
            interpolateAngle(transformable->getRotation().y, yaw, rotationSpeed * deltaTime),
            interpolateAngle(transformable->getRotation().z, roll, rotationSpeed * deltaTime)
        );

        transformable->setRotation(newRot);
    }

    void setSpeed(float moveSpeed, float turnSpeed) {
        speed = moveSpeed;
        rotationSpeed = turnSpeed;
    }

    void setArrivalThreshold(float threshold) {
        arrivalThreshold = threshold;
    }

private:
    Transformable* transformable;
    std::vector<glm::vec3> waypoints;
    size_t currentTarget;
    float speed;
    float rotationSpeed;
    float arrivalThreshold;
    int loopCount;

    float interpolateAngle(float from, float to, float maxStep) {
        float delta = fmodf(to - from + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();
        delta = glm::clamp(delta, -maxStep, maxStep);
        return from + delta;
    }
};

#endif
