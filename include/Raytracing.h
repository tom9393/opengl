#pragma once

#include "Camera.h"
#include "Ray.h"
#include "AABB.h"
#include "mesh.h"
#include "model.h"

class Raytracing
{
    public:
    static Ray CastRayFromMouse(float mouseX, float mouseY, float screenWidth, float screenHeight, std::shared_ptr<Camera> camera)
    {
        // Normalized Device Coordinates
        float x = (2.0f * mouseX) / screenWidth - 1.0f;
        float y = 1.0f - (2.0f * mouseY) / screenHeight;
        glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

        glm::mat4 proj = camera->GetProjectionMatrix();
        glm::mat4 view = camera->GetViewMatrix();

        // Eye space
        glm::vec4 rayEye = glm::inverse(proj) * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

        // World space
        glm::vec3 rayWorld = glm::vec3(glm::inverse(view) * rayEye);
        rayWorld = glm::normalize(rayWorld);

        return { camera->GetPosition(), rayWorld };
    }

    static bool RayIntersectsAABB(const Ray& ray, const AABB& box, float& t)
    {
        float tmin = (box.min.x - ray.origin.x) / ray.direction.x;
        float tmax = (box.max.x - ray.origin.x) / ray.direction.x;
        if (tmin > tmax) std::swap(tmin, tmax);

        float tymin = (box.min.y - ray.origin.y) / ray.direction.y;
        float tymax = (box.max.y - ray.origin.y) / ray.direction.y;
        if (tymin > tymax) std::swap(tymin, tymax);

        if ((tmin > tymax) || (tymin > tmax))
            return false;

        tmin = std::max(tmin, tymin);
        tmax = std::min(tmax, tymax);

        float tzmin = (box.min.z - ray.origin.z) / ray.direction.z;
        float tzmax = (box.max.z - ray.origin.z) / ray.direction.z;
        if (tzmin > tzmax) std::swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax))
            return false;

        t = tmin;
        return true;
    }

    static void TrySelectMesh(float mouseX, float mouseY, float screenWidth, float screenHeight, std::shared_ptr<Camera> camera, std::shared_ptr<Model> model, std::shared_ptr<Mesh> &selected)
    {
        Ray ray = CastRayFromMouse(mouseX, mouseY, screenWidth, screenHeight, camera);

        float closestT = std::numeric_limits<float>::max();
        selected = nullptr;

        for (auto& mesh : model->GetMeshes())
        {
            AABB aabb = mesh->GetTransformedAABB();
            float t;
            if (RayIntersectsAABB(ray, aabb, t))
            {
                if (t < closestT)
                {
                    closestT = t;
                    selected = mesh;
                }
            }
        }
    }
};
