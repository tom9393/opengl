#ifndef OBJ
#define OBJ

#include "drawable.h"
#include "shader.h"
#include <glm/fwd.hpp>

struct Transform {
    glm::vec3 position;
    glm::vec3 rotation; // Euler angles in radians: X = pitch, Y = yaw, Z = roll
    glm::vec3 scale;

    Transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) : position(position), rotation(rotation), scale(scale) {}

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
    glm::mat4 getRotatedMatrixAround(const glm::vec3& point, const glm::vec3& axis, float angleRad) const {
        glm::mat4 model = getMatrix();
        glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -point);
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), angleRad, axis);
        glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), point);

        return translateBack * rotate * translateToOrigin * model;
    }

    // --- Scale ---
    void setScale(const glm::vec3& s) { scale = s; }
    void scaleBy(const glm::vec3& factor) { scale *= factor; }

    // --- Reset all transforms ---
    void reset() {
        position = glm::vec3(0.0f);
        rotation = glm::vec3(0.0f);
        scale = glm::vec3(1.0f);
    }
};

class Obj : public IDrawable
{
public:
    Obj(IDrawable &drawable, glm::vec3 location = glm::vec3(0.0f), glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f)) : drawable(&drawable), location(Transform(location, rotation, scale)), velocity(glm::vec3(0.0f)), angularVelocity(glm::vec3(0.0f)) {};
    void setLocation(glm::vec3 &loc);
    void rotate(glm::vec3 &target);
    
    glm::mat4 getMatrix();
    Transform* getTransform();
    void draw(Shader &shader);
    void update(float deltaTime);
    void setVelocity(const glm::vec3& v) { velocity = v; }
    void setAngularVelocity(const glm::vec3& av) { angularVelocity = av; }

    Transform location;
    glm::vec3 velocity;
    glm::vec3 angularVelocity;

private:
    IDrawable* drawable;
};

#endif
