#include "MaterialBindings.h"

#include "pch.h"
#include "Asset/Material.h"

namespace Engine {

    void MaterialBindings::Register(sol::state& state)
    {
        // Scalar/color material properties scripts may animate (textures are not exposed).
        state.new_enum("MaterialProperty",
            "AlbedoTint", MaterialType::AlbedoTint,
            "Emission", MaterialType::Emission,
            "RoughnessMultiplier", MaterialType::RoughnessMultiplier,
            "MetallicMultiplier", MaterialType::MetallicMultiplier,
            "Transparency", MaterialType::Transparency
        );

        // Obtained via Entity:GetMaterial(). Materials are shared assets: a change
        // affects every entity rendering with the material.
        state.new_usertype<Material>("Material",
            "Set", sol::overload(
                [](Material& material, MaterialType type, float value) { material.m_Properties[type] = value; },
                [](Material& material, MaterialType type, const glm::vec4& value) { material.m_Properties[type] = value; }
            ),
            "GetFloat", [](Material& material, MaterialType type) -> sol::optional<float> {
                auto it = material.m_Properties.find(type);
                if (it == material.m_Properties.end() || !std::holds_alternative<float>(it->second)) return sol::nullopt;
                return std::get<float>(it->second);
            },
            "GetVec4", [](Material& material, MaterialType type) -> sol::optional<glm::vec4> {
                auto it = material.m_Properties.find(type);
                if (it == material.m_Properties.end() || !std::holds_alternative<glm::vec4>(it->second)) return sol::nullopt;
                return std::get<glm::vec4>(it->second);
            }
        );
    }

}
