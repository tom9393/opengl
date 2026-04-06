#include "Texture.h"

namespace Engine {

    class Material : public Asset
    {
        public:
        Shared<Texture2D> AlbedoMap = nullptr;
        Shared<Texture2D> NormalMap = nullptr;
        Shared<Texture2D> MetallicMap = nullptr;
        Shared<Texture2D> RoughnessMap = nullptr;
        Shared<Texture2D> AOMap = nullptr;

        // Scalar multipliers / fallback colors
        glm::vec4 AlbedoTint = glm::vec4(1.0f);
        float MetallicMultiplier = 0.0f;
        float RoughnessMultiplier = 0.5f;
        float Emission = 0.0f;
    };
}