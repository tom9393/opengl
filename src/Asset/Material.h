#pragma once

#include "MaterialSpecification.h"
#include "DefaultTextures.h"
#include "Shader.h"
#include "Texture.h"

namespace Engine {

    enum class BlendMode { Opaque, Transparent, Additive };

    class Material : public Asset
    {
        public:
        Shared<Shader> TargetShader;

        // Todo: implement these
        BlendMode mode = BlendMode::Opaque;
        bool DepthWrite = true;
        bool DoubleSided = false;

        std::unordered_map<MaterialType, PropertyValue> m_Properties; // Material type and data
        std::unordered_map<MaterialType, std::string> m_Bindings; // What uniforms the shader has available for binding (e.g. MaterialType::Albedo -> pbr.albedoMap)

        void Add(const MaterialProperty& property)
        {
            m_Properties[property.Type] = property.Value;
        }

        void Remove(const MaterialType key)
        {
            m_Properties.erase(key);
        }

        void Bind() const
        {
            TargetShader->Bind();

            uint32_t slot = 0;
            for (const auto& [key, uniformName] : m_Bindings)
            {
                if (IsTextureType(key))
                    BindTexture(key, uniformName, slot);
                else
                    BindScalar(key, uniformName);
            }
        }

        void BindTexture(MaterialType key, const std::string& uniformName, uint32_t& slot) const
        {
            auto it = m_Properties.find(key);
            Shared<Texture2D> texture = nullptr;

            if (it != m_Properties.end() && std::holds_alternative<Shared<Texture2D>>(it->second)) {
                texture = std::get<Shared<Texture2D>>(it->second);
            }

            if (!texture) {
                texture = GetDefaultFallback(key);
            }

            texture->Bind(slot);
            TargetShader->SetInt(uniformName, slot);
            slot++;
        }

        // Scalar as in non-texture i.e. RGBA
        void BindScalar(MaterialType key, const std::string& uniformName) const
        {
            auto it = m_Properties.find(key);
            if (it == m_Properties.end()) return;

            const PropertyValue& value = it->second;

            if (std::holds_alternative<float>(value))
                TargetShader->SetFloat(uniformName, std::get<float>(value));
            else if (std::holds_alternative<int>(value))
                TargetShader->SetInt(uniformName, std::get<int>(value));
            else if (std::holds_alternative<glm::vec4>(value))
                TargetShader->SetVec4(uniformName, std::get<glm::vec4>(value));
        }

        static bool IsTextureType(MaterialType type)
        {
            switch (type) {
                case MaterialType::Albedo:
                case MaterialType::Normal:
                case MaterialType::Metallic:
                case MaterialType::Roughness:
                case MaterialType::AO:
                case MaterialType::Height:
                case MaterialType::Emissive:
                case MaterialType::Diffuse:
                case MaterialType::Specular:
                    return true;
                default:
                    return false;
            }
        }

        Shared<Texture2D> GetDefaultFallback(MaterialType type) const
        {
            switch(type) {
                case MaterialType::Normal:   return DefaultTextures::GetNormalTexture();
                case MaterialType::Metallic: return DefaultTextures::GetBlackTexture();
                default:                     return DefaultTextures::GetWhiteTexture();
            }
        }
    };
}