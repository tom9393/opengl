#ifndef TRANSFORM
#define TRANSFORM

#include "transformable.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform
{
public:
    Transform() = delete;
    // --- Position ---
    static void setPosition(Transformable &transformable, const glm::vec3& pos) {
        transformable.setPosition(pos);
    }
    static void move(Transformable &transformable, const glm::vec3& delta) {
        glm::vec3 prevPos = transformable.getPosition();
        transformable.setPosition(prevPos + delta);
    }

    // --- Rotation ---
    static void setRotation(Transformable &transformable, const glm::vec3& rot) {
        transformable.setRotation(rot);
    }
    static void rotate(Transformable &transformable, const glm::vec3& delta) {
        glm::vec3 prevRot = transformable.getRotation();
        transformable.setRotation(prevRot + delta);
    }

    // Arbitrary axis rotation around a point
    static glm::mat4 getRotatedMatrixAround(Transformable &transformable, const glm::vec3& point, const glm::vec3& axis, float angleRad) {
        glm::mat4 model = transformable.getMatrix();
        glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -point);
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), angleRad, axis);
        glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), point);

        return translateBack * rotate * translateToOrigin * model;
    }

    static void rotateTowards(Transformable &transformable, const glm::vec3& targetPosition)
    {
        glm::vec3 direction = glm::normalize(targetPosition - transformable.getPosition());

        // Compute yaw (rotation around Y axis)
        float yaw = std::atan2(direction.x, direction.z);

        // Compute pitch (rotation around X axis)
        float horizontalDistance = std::sqrt(direction.x * direction.x + direction.z * direction.z);
        float pitch = std::atan2(-direction.y, horizontalDistance); // negative to look "up" when Y increases

        // Roll is left unchanged (assumed to be 0)
        transformable.setRotation(glm::vec3(pitch, yaw, 0.0f));
    }

    static void rotateTowards(Transformable &target, Transformable &other)
    {
        rotateTowards(target, other.getPosition());
    }

    static glm::vec3 getDir(Transformable &transformable, const glm::vec3 &target)
    {
        return glm::normalize(target - transformable.getPosition());
    }

    static glm::vec3 reverseDir(Transformable &transformable, const glm::vec3 &target)
    {
        glm::mat4 mat = glm::lookAt(transformable.getPosition(), target, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 positionInversed = glm::vec3(glm::inverse(mat)[3]);
        return positionInversed;
    }

    static glm::vec3 getForwardDirection(Transformable &t) {
        glm::vec3 rot = t.getRotation();
        float yaw = rot.y;
        float pitch = rot.x;

        glm::vec3 forward;
        forward.x = sin(yaw) * cos(pitch);
        forward.y = -sin(pitch);
        forward.z = cos(yaw) * cos(pitch);

        return glm::normalize(forward);
    }

    // --- Scale ---
    static void setScale(Transformable &transformable, const glm::vec3& s) {
        transformable.setScale(s);
    }
    static void scaleBy(Transformable &transformable, const glm::vec3& factor) {
        glm::vec3 prevScale = transformable.getScale();
        transformable.setScale(prevScale * factor);
    }
    // --- Velocities ---
    static void setVelocity(Transformable &transformable, const glm::vec3& v) {
        transformable.setVelocity(v);
    }
    static void setAngularVelocity(Transformable &transformable, const glm::vec3& av) {
        transformable.setAngularVelocity(av);
    }

    // --- Reset all transforms ---
    static void reset(Transformable &transformable) {
        transformable.setPosition(glm::vec3(0.0f));
        transformable.setRotation(glm::vec3(0.0f));
        transformable.setScale(glm::vec3(1.0f));
    }
    
    static void update(Transformable &transformable, float deltaTime) {
        move(transformable, transformable.getVelocity() + deltaTime);
        rotate(transformable, transformable.getAngularVelocity() + deltaTime);
    }

    static void updateRotation(Transformable &transformable, float deltaTime) {
        rotate(transformable, transformable.getAngularVelocity() + deltaTime);
    }

    static void updateMovement(Transformable &transformable, float deltaTime) {
        move(transformable, transformable.getVelocity() + deltaTime);
    }

    static void rotateX(Transformable &transformable, float angleRad) {
        glm::vec3 prevRot = transformable.getRotation();
        prevRot.x += angleRad;
        transformable.setRotation(prevRot);
    }

    static void rotateY(Transformable &transformable, float angleRad) {
        glm::vec3 prevRot = transformable.getRotation();
        prevRot.y += angleRad;
        transformable.setRotation(prevRot);
    }

    static void rotateZ(Transformable &transformable, float angleRad) {
        glm::vec3 prevRot = transformable.getRotation();
        prevRot.z += angleRad;
        transformable.setRotation(prevRot);
    }
};

#endif
