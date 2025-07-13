#ifndef PATH
#define PATH
#define GLM_ENABLE_EXPERIMENTAL

#include "obj.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <vector>
#include <cmath>


class Path {
public:
    Path(Transform *transform, float moveSpeed = 1.0f, float turnSpeed = 1.0f) : transform(transform), speed(moveSpeed), rotationSpeed(turnSpeed), currentTarget(0), arrivalThreshold(0.1f), loopCount(0) {}

    void setPath(const std::vector<glm::vec3>& points) {
        waypoints = points;
        currentTarget = 0;
        loopCount = 0;
    }

    void update(float deltaTime, int(*cb)(int)) {
        if (waypoints.empty()) return;

        glm::vec3 target = waypoints[currentTarget];
        glm::vec3 direction = target - transform->position;
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
        transform->move(dirNorm * speed * deltaTime);

        // Compute full Euler angles: yaw (Y), pitch (X)
        float yaw = atan2(dirNorm.x, dirNorm.z);
        float pitch = -asin(dirNorm.y); // assuming y-up
        float roll = 0.0f; // flat ground; could be extended to lean into curves if needed

        // Smooth interpolation
        transform->rotation.y = interpolateAngle(transform->rotation.y, yaw, rotationSpeed * deltaTime);
        transform->rotation.x = interpolateAngle(transform->rotation.x, pitch, rotationSpeed * deltaTime);
        transform->rotation.z = interpolateAngle(transform->rotation.z, roll, rotationSpeed * deltaTime);
    }

    void setSpeed(float moveSpeed, float turnSpeed) {
        speed = moveSpeed;
        rotationSpeed = turnSpeed;
    }

    void setArrivalThreshold(float threshold) {
        arrivalThreshold = threshold;
    }

private:
    Transform* transform;
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
