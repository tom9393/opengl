#include "Texture.h"
#include "Shader.h"

namespace Engine {

    enum class BlendMode { Opaque, Transparent, Additive };

    class Material : public Asset
    {
        public:
        Shared<Shader> TargetShader;

        BlendMode mode = BlendMode::Opaque;
        bool DepthWrite = true;
        bool DoubleSided = false;

        void SetFloat(const std::string& name, float value) { m_Floats[name] = value; }
        void SetBool(const std::string& name, bool value) { m_Bools[name] = value; }
        void SetVec4(const std::string& name, const glm::vec4& value) { m_Vec4s[name] = value; }
        void SetTexture(const std::string& name, Shared<Texture2D> texture) { m_Textures[name] = texture; }

        void Bind() const {
            TargetShader->Bind();

            for (const auto& [name, val] : m_Floats) TargetShader->SetFloat(name, val);
            for (const auto& [name, val] : m_Bools) TargetShader->SetBool(name, val);
            for (const auto& [name, val] : m_Vec4s)  TargetShader->SetVec4(name, val);
            
            uint32_t slot = 0;
            for (const auto& [name, tex] : m_Textures) {
                if (tex) {
                    tex->Bind(slot);
                    TargetShader->SetInt(name, slot);
                    slot++;
                }
            }
        }

        std::unordered_map<std::string, float> m_Floats;
        std::unordered_map<std::string, bool> m_Bools;
        std::unordered_map<std::string, glm::vec4> m_Vec4s;
        std::unordered_map<std::string, Shared<Texture2D>> m_Textures;
    };
}