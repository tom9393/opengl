#pragma once

#include "../Core/UUID.h"

namespace Engine {

    enum class AssetType
    {
        None = 0,
        Texture2D,
        Texture3D,
        Scene,
        Model,
        Mesh,
        Material
    };

    struct AssetMetadata {
        UUID Handle = 0;
        AssetType Type = AssetType::None;
        std::string FilePath;
        // Optional: Add SubmeshIndex if you want to reference specific meshes inside a larger Assimp model
        uint32_t SubmeshIndex = 0; 
        
        bool IsValid() const { return Handle != 0; }
    };

    class Asset
    {
        public:
        UUID Handle;
        AssetType GetType() { return m_Type; };

        protected:
        bool m_Loaded = false;
        AssetType m_Type;
    };
}
