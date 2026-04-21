#pragma once

#include "Texture.h"
#include "../pch.h"

namespace Engine {

    enum class TextureType
    {
        None = 0,
        Albedo,    // PBR: Base color
        Normal,    // PBR: Surface detail
        Metallic,  // PBR: Metalness
        Roughness, // PBR: Micro-surface detail
        AO,        // PBR: Ambient Occlusion
        Height,    // Often used for parallax
        Diffuse,   // Legacy
        Specular,  // Legacy
        Emissive,
        Transparency
    };

    enum class PropertyKey
    {
        AlbedoTint,
        Emission,
        RoughnessMultiplier,
        MetallicMultiplier,

        Map
    };

    struct TextureMap
    {
        TextureType Type;
        Shared<Texture2D> MapTexture;

        TextureMap(TextureType type, Shared<Texture2D> &texture) : Type(type), MapTexture(texture) {}
    };

    using PropertyValue = std::variant<float, int, glm::vec4, TextureMap>;

    struct MaterialProperty
    {
        public:
        PropertyKey Type; 
        PropertyValue Value;

        MaterialProperty(PropertyKey type, PropertyValue value) : Type(type), Value(value) {}
    };

}