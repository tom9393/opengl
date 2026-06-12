#pragma once

#include "pch.h"
#include "Texture.h"

namespace Engine {

    enum class MaterialType
    {
        None = 0,
        
        // Maps
        Albedo,
        Normal,
        Metallic,
        Roughness,
        AO,
        Height,
        Emissive,

        Diffuse,
        Specular,

        // Multipliers
        Transparency,
        
        AlbedoTint,
        Emission,
        RoughnessMultiplier,
        MetallicMultiplier
    };

    using PropertyValue = std::variant<float, int, glm::vec4, Shared<Texture2D>>;

    struct MaterialProperty
    {
        MaterialType Type; 
        PropertyValue Value;

        MaterialProperty(MaterialType type, PropertyValue value) : Type(type), Value(value) {}
    };

}