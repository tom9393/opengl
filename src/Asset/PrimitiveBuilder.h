#pragma once
#include "../../pch.h"
#include "Mesh.h"
#include "AssetManager.h"
#include <glm/glm.hpp>

namespace Engine {

    enum class PrimitiveType
    {
        Plane,
        Cube,
        Cylinder,
        Capsule,
        Sphere
    };

    static const PrimitiveType AllPrimitives[] = {
        PrimitiveType::Plane,
        PrimitiveType::Cube,
        PrimitiveType::Cylinder,
        PrimitiveType::Capsule,
        PrimitiveType::Sphere
    };

    static const char* PrimitiveNames[] = {
        "Plane",
        "Cube",
        "Cylinder",
        "Capsule",
        "Sphere"
    };

    struct PrimitiveSpecification
    {
        PrimitiveType Type;
        uint32_t Segments[3] = {1, 1, 1};
        glm::vec3 Size = {1.0f, 1.0f, 1.0f};
        bool CalculateNormals = true;
        bool CalculateTangents = true;
        bool CalculateBitangents = true;
    };

    enum Axis
    {
        x,
        y,
        z,
    };

    class PrimitiveBuilder
    {
    public:
        static Shared<Mesh> CreatePrimitive(PrimitiveSpecification spec, const std::string& name = "Primitive");

    private:

        static inline std::unordered_map<PrimitiveType, Shared<Mesh>> m_ReadyTypes;

        static void BuildPlane(const PrimitiveSpecification& spec, std::vector<float>& vertices, std::vector<uint32_t>& indices);
        static void BuildCube(const PrimitiveSpecification& spec, std::vector<float>& vertices, std::vector<uint32_t>& indices);
        static void BuildSphere(const PrimitiveSpecification& spec, std::vector<float>& vertices, std::vector<uint32_t>& indices);
        static void BuildCylinder(const PrimitiveSpecification& spec, std::vector<float>& vertices, std::vector<uint32_t>& indices);
        static void BuildCapsule(const PrimitiveSpecification& spec, std::vector<float>& vertices, std::vector<uint32_t>& indices);
    };

}