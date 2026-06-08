#pragma once

#include "Renderer.h"
#include "Asset/Asset.h"

#include <unordered_map>

namespace Engine {

    class LoadedAssets
    {
        public:
        void OnImGuiRender();

    private:
        std::unordered_map<AssetType, bool> m_VisibleTypes = {
            { AssetType::Texture2D, true },
            { AssetType::Texture3D, true },
            { AssetType::Scene,     true },
            { AssetType::Prefab,    true },
            { AssetType::Mesh,      true },
            { AssetType::Material,  true },
            { AssetType::LuaScript, true },
            { AssetType::Audio,     true },
        };
    };
}