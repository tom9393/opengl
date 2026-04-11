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
        uint32_t Index = 0; 
        std::vector<AssetMetadata> Children;

        bool HasChildren() { return !Children.empty(); }
        
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
