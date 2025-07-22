#ifndef TRANSFORMABLE
#define TRANSFORMABLE

#include "../pch.h"

class Transformable
{
public:
    Transformable(glm::vec3 position, glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f), glm::vec3 velocity = glm::vec3(0.0f), glm::vec3 angularVelocity = glm::vec3(0.0f)) : position(position), rotation(rotation), scale(scale), velocity(velocity), angularVelocity(angularVelocity) {}
    Transformable(Transformable &transform) : position(transform.position), rotation(transform.rotation), scale(transform.scale), velocity(transform.velocity), angularVelocity(transform.angularVelocity) {}

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
    glm::vec3 getPosition()
    {
        return position;
    }

    void setPosition(const glm::vec3 pos)
    {
        position = pos;
    }

    glm::vec3 getRotation()
    {
        return rotation;
    }

    void setRotation(const glm::vec3 rot)
    {
        rotation = rot;
    }

    glm::vec3 getScale()
    {
        return scale;
    }

    void setScale(const glm::vec3 s)
    {
        scale = s;
    }

    glm::vec3 getVelocity()
    {
        return velocity;
    }

    void setVelocity(const glm::vec3 v)
    {
        velocity = v;
    }

    glm::vec3 getAngularVelocity()
    {
        return angularVelocity;
    }

    void setAngularVelocity(const glm::vec3 av)
    {
        angularVelocity = av;
    }

private:
    glm::vec3 position;
    glm::vec3 rotation; // Euler angles in radians: X = pitch, Y = yaw, Z = roll
    glm::vec3 scale;
    glm::vec3 velocity;
    glm::vec3 angularVelocity;
};

#endif
