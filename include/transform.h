#ifndef TRANSFORM
#define TRANSFORM

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform
{
public:
    glm::vec3 position;
    glm::vec3 rotation; // Euler angles in radians: X = pitch, Y = yaw, Z = roll
    glm::vec3 scale;
    glm::vec3 velocity;
    glm::vec3 angularVelocity;

    Transform(glm::vec3 position, glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f), glm::vec3 velocity = glm::vec3(0.0f), glm::vec3 angularVelocity = glm::vec3(0.0f)) : position(position), rotation(rotation), scale(scale), velocity(velocity), angularVelocity(angularVelocity) {}
    Transform(Transform &transform) : position(transform.position), rotation(transform.rotation), scale(transform.scale), velocity(transform.velocity), angularVelocity(transform.angularVelocity) {}

    // Constructs transformation matrix: Translation * Rotation(ZYX) * Scale
    glm::mat4 getMatrix() const {
        glm::mat4 mat = glm::mat4(1.0f);

        // Apply translation
        mat = glm::translate(mat, position);

        // Apply rotation (roll → yaw → pitch)
        mat = glm::rotate(mat, rotation.z, glm::vec3(0, 0, 1)); // Roll
        mat = glm::rotate(mat, rotation.y, glm::vec3(0, 1, 0)); // Yaw
        mat = glm::rotate(mat, rotation.x, glm::vec3(1, 0, 0)); // Pitch

        // Apply scale
        mat = glm::scale(mat, scale);

        return mat;
    }

    // --- Position ---
    void setPosition(const glm::vec3& pos) { position = pos; }
    void move(const glm::vec3& delta) { position += delta; }

    // --- Rotation ---
    void setRotation(const glm::vec3& rot) { rotation = rot; }
    void rotate(const glm::vec3& delta) { rotation += delta; }

    // Arbitrary axis rotation around a point
    glm::mat4 getRotatedMatrixAround(const glm::vec3& point, const glm::vec3& axis, float angleRad) {
        glm::mat4 model = getMatrix();
        glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -point);
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), angleRad, axis);
        glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), point);

        return translateBack * rotate * translateToOrigin * model;
    }

    void rotateTowards(const glm::vec3& targetPosition)
    {
        glm::vec3 direction = glm::normalize(targetPosition - position);

        // Compute yaw (rotation around Y axis)
        float yaw = std::atan2(direction.x, direction.z);

        // Compute pitch (rotation around X axis)
        float horizontalDistance = std::sqrt(direction.x * direction.x + direction.z * direction.z);
        float pitch = std::atan2(-direction.y, horizontalDistance); // negative to look "up" when Y increases

        // Roll is left unchanged (assumed to be 0)
        rotation = glm::vec3(pitch, yaw, 0.0f);
    }

    glm::vec3 getDir(const glm::vec3 &target)
    {
        return glm::normalize(target - position);
    }

    glm::vec3 reverseDir(const glm::vec3 &target)
    {
        glm::mat4 mat = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 positionInversed = glm::vec3(glm::inverse(mat)[3]);
        return positionInversed;
    }

    glm::vec3 getForwardDirection() const {
        glm::mat4 rot = glm::mat4(1.0f);
        rot = glm::rotate(rot, rotation.z, glm::vec3(0, 0, 1));
        rot = glm::rotate(rot, rotation.y, glm::vec3(0, 1, 0));
        rot = glm::rotate(rot, rotation.x, glm::vec3(1, 0, 0));
        return glm::normalize(glm::vec3(rot * glm::vec4(0, 0, -1, 0)));
    }

    // --- Scale ---
    void setScale(const glm::vec3& s) { scale = s; }
    void scaleBy(const glm::vec3& factor) { scale *= factor; }
    // --- Velocities ---
    void setVelocity(const glm::vec3& v) { velocity = v; }
    void setAngularVelocity(const glm::vec3& av) { angularVelocity = av; }

    // --- Reset all transforms ---
    void reset() {
        position = glm::vec3(0.0f);
        rotation = glm::vec3(0.0f);
        scale = glm::vec3(1.0f);
    }

    void update(float deltaTime) {
        move(velocity + deltaTime);
        rotate(angularVelocity + deltaTime);
    }

    void updateRotation(float deltaTime) {
        rotate(angularVelocity + deltaTime);
    }

    void updateMovement(float deltaTime) {
        move(velocity + deltaTime);
    }

    void rotateX(float angleRad) {
    rotation.x += angleRad;
    }

    void rotateY(float angleRad) {
        rotation.y += angleRad;
    }

    void rotateZ(float angleRad) {
        rotation.z += angleRad;
    }
};

#endif
