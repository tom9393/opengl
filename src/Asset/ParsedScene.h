#pragma once

#include "pch.h"
#include "MaterialSpecification.h"
#include "TextureImporter.h"
#include "Scene/Components.h"

namespace Engine
{

    struct GeometrySpecification
    {
        bool CalculateNormals;
        bool CalculateTangents;
        bool CalculateBitangents;
        bool ImportTextures;
        bool ImportAnimations;
    };

    struct ParsedMesh {
        std::string Name;
        std::vector<float> Vertices;
        std::vector<unsigned int> Indices;
        uint32_t MaterialIndex = 0;
        glm::vec3 MinExtents;
        glm::vec3 MaxExtents;
    };

    struct ParsedMaterial {
        std::string Name;
        glm::vec4 AlbedoTint;
        float Emission;
        float Roughness;
        float Metallic;
        std::unordered_map<MaterialType, std::string> TexturePaths;
        std::unordered_map<MaterialType, TextureLoadData> EmbeddedTextures;
    };

    struct ParsedLight {
        std::string Name;
        LightComponent::LightType Type = LightComponent::LightType::Point;
        glm::vec3 Color = glm::vec3(1.0f);
        float Intensity = 1.0f;
        bool CastShadows = true;
        float InnerCutOff = 0.0f; // cosine of the inner cone half-angle (cone lights only)
        float OuterCutOff = 0.0f; // cosine of the outer cone half-angle (cone lights only)
    };

    struct ParsedCamera {
        std::string Name;
        bool Orthographic = false;
        float VerticalFOV = glm::radians(45.0f);
        float OrthographicSize = 10.0f;
        float NearClip = 0.01f;
        float FarClip = 1000.0f;
        bool Primary = false;
    };

    // A node in the parsed scene tree. Carries everything needed to build its
    // entity directly: transform, an optional light/camera, and its meshes.
    // Each mesh's MaterialIndex refers into the scene-global material list.
    struct ParsedSceneNode {
        std::string Name;
        glm::vec3 Translation;
        glm::vec3 Rotation;
        glm::vec3 Scale;
        std::vector<ParsedMesh> Meshes;
        std::optional<ParsedLight> Light;
        std::optional<ParsedCamera> Camera;
        std::vector<ParsedSceneNode> Children;
    };

    struct ParsedScene {
        std::string Name;
        std::string Directory;
        // Shared by all meshes in the scene; indexed by ParsedMesh::MaterialIndex.
        std::vector<ParsedMaterial> Materials;
        ParsedSceneNode RootNode;
    };

}
