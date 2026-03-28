#pragma once

namespace Engine
{

    class MathTools
    {
        public:
        static bool DecomposeMatrix(const glm::mat4 &transform, glm::vec3 &translation, glm::vec3 &rotation, glm::vec3 &scale);
    };

    struct AABB {
        glm::vec3 Min{ 0.0f, 0.0f, 0.0f };
        glm::vec3 Max{ 0.0f, 0.0f, 0.0f };
    };

    struct Plane {
        glm::vec3 Normal = { 0.f, 1.f, 0.f };
        float Distance = 0.f; // Distance from origin to the plane

        // Normalize the plane equation
        void Normalize() {
            float mag = glm::length(Normal);
            Normal /= mag;
            Distance /= mag;
        }
    };

    struct Frustum {
        Plane Left, Right, Bottom, Top, Near, Far;
    };

    inline Frustum CreateFrustumFromMatrix(const glm::mat4& viewProj)
    {
        Frustum frustum;

        // Left Plane
        frustum.Left.Normal.x = viewProj[0][3] + viewProj[0][0];
        frustum.Left.Normal.y = viewProj[1][3] + viewProj[1][0];
        frustum.Left.Normal.z = viewProj[2][3] + viewProj[2][0];
        frustum.Left.Distance = viewProj[3][3] + viewProj[3][0];

        // Right Plane
        frustum.Right.Normal.x = viewProj[0][3] - viewProj[0][0];
        frustum.Right.Normal.y = viewProj[1][3] - viewProj[1][0];
        frustum.Right.Normal.z = viewProj[2][3] - viewProj[2][0];
        frustum.Right.Distance = viewProj[3][3] - viewProj[3][0];

        // Bottom Plane
        frustum.Bottom.Normal.x = viewProj[0][3] + viewProj[0][1];
        frustum.Bottom.Normal.y = viewProj[1][3] + viewProj[1][1];
        frustum.Bottom.Normal.z = viewProj[2][3] + viewProj[2][1];
        frustum.Bottom.Distance = viewProj[3][3] + viewProj[3][1];

        // Top Plane
        frustum.Top.Normal.x = viewProj[0][3] - viewProj[0][1];
        frustum.Top.Normal.y = viewProj[1][3] - viewProj[1][1];
        frustum.Top.Normal.z = viewProj[2][3] - viewProj[2][1];
        frustum.Top.Distance = viewProj[3][3] - viewProj[3][1];

        // Near Plane
        frustum.Near.Normal.x = viewProj[0][3] + viewProj[0][2];
        frustum.Near.Normal.y = viewProj[1][3] + viewProj[1][2];
        frustum.Near.Normal.z = viewProj[2][3] + viewProj[2][2];
        frustum.Near.Distance = viewProj[3][3] + viewProj[3][2];

        // Far Plane
        frustum.Far.Normal.x = viewProj[0][3] - viewProj[0][2];
        frustum.Far.Normal.y = viewProj[1][3] - viewProj[1][2];
        frustum.Far.Normal.z = viewProj[2][3] - viewProj[2][2];
        frustum.Far.Distance = viewProj[3][3] - viewProj[3][2];

        // Normalize all planes
        frustum.Left.Normalize();   frustum.Right.Normalize();
        frustum.Bottom.Normalize(); frustum.Top.Normalize();
        frustum.Near.Normalize();   frustum.Far.Normalize();

        return frustum;
    }

    inline bool IsAABBInFrustum(const Frustum& frustum, const AABB& localAABB, const glm::mat4& transform)
    {
        // Get the global center and extents of the AABB
        glm::vec3 center = (localAABB.Max + localAABB.Min) * 0.5f;
        glm::vec3 extents = localAABB.Max - center;

        // Transform center to world space
        glm::vec3 worldCenter = glm::vec3(transform * glm::vec4(center, 1.0f));

        // Transform extents (needs only rotation/scale, not translation)
        glm::vec3 right = glm::vec3(transform[0]) * extents.x;
        glm::vec3 up = glm::vec3(transform[1]) * extents.y;
        glm::vec3 forward = glm::vec3(transform[2]) * extents.z;

        // Calculate the absolute world extents (radius) on each axis
        float rX = std::abs(right.x) + std::abs(up.x) + std::abs(forward.x);
        float rY = std::abs(right.y) + std::abs(up.y) + std::abs(forward.y);
        float rZ = std::abs(right.z) + std::abs(up.z) + std::abs(forward.z);
        glm::vec3 worldExtents(rX, rY, rZ);

        // Check against all 6 planes
        const Plane* planes[6] = { &frustum.Left, &frustum.Right, &frustum.Bottom, &frustum.Top, &frustum.Near, &frustum.Far };
        
        for (int i = 0; i < 6; ++i)
        {
            const Plane& p = *planes[i];
            
            // Calculate distance from center to plane
            float d = glm::dot(worldCenter, p.Normal) + p.Distance;
            
            // Calculate the maximum possible radius along the plane normal
            float r = worldExtents.x * std::abs(p.Normal.x) +
                      worldExtents.y * std::abs(p.Normal.y) +
                      worldExtents.z * std::abs(p.Normal.z);

            // If the distance is less than the negative radius, it is completely outside
            if (d < -r)
                return false; 
        }

        return true; // AABB is inside or intersecting the frustum
    }

    inline glm::vec3 ScreenPosToWorldRay(
        float mouseX, float mouseY, 
        float viewportWidth, float viewportHeight, 
        const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) 
    {
        // 1. Convert Screen coordinates to Normalized Device Coordinates (NDC) [-1 to 1]
        float x = (2.0f * mouseX) / viewportWidth - 1.0f;
        float y = 1.0f - (2.0f * mouseY) / viewportHeight; // Y is inverted in screen space!

        // 2. Clip Space
        glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

        // 3. View Space (Reverse Projection)
        glm::vec4 rayView = glm::inverse(projectionMatrix) * rayClip;
        rayView = glm::vec4(rayView.x, rayView.y, -1.0f, 0.0f); // Set Z to point forwards, W to 0 (it's a vector, not a point)

        // 4. World Space (Reverse View)
        glm::vec3 rayWorld = glm::vec3(glm::inverse(viewMatrix) * rayView);
        
        return glm::normalize(rayWorld);
    }

    inline bool RayIntersectsAABB(
        const glm::vec3& rayOrigin, const glm::vec3& rayDir, 
        const AABB& aabb, const glm::mat4& entityTransform, 
        float& outIntersectionDistance) 
    {
        // 1. Transform the Ray into the Entity's Local Space
        glm::mat4 inverseTransform = glm::inverse(entityTransform);
        
        glm::vec3 localRayOrigin = glm::vec3(inverseTransform * glm::vec4(rayOrigin, 1.0f));
        // W is 0.0f for direction vectors so they aren't affected by translation
        glm::vec3 localRayDir = glm::normalize(glm::vec3(inverseTransform * glm::vec4(rayDir, 0.0f))); 

        // 2. The "Slab" Intersection Math
        glm::vec3 invDir = 1.0f / localRayDir;
        
        glm::vec3 t0 = (aabb.Min - localRayOrigin) * invDir;
        glm::vec3 t1 = (aabb.Max - localRayOrigin) * invDir;

        glm::vec3 tMin = glm::min(t0, t1);
        glm::vec3 tMax = glm::max(t0, t1);

        float tNear = glm::max(glm::max(tMin.x, tMin.y), tMin.z);
        float tFar = glm::min(glm::min(tMax.x, tMax.y), tMax.z);

        // If tNear is greater than tFar, the ray missed the box
        // If tFar is negative, the box is completely behind the camera
        if (tNear >= tFar || tFar < 0.0f) 
            return false;

        outIntersectionDistance = tNear;
        return true;
    }

};
