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
            { AssetType::Texture2D, false },
            { AssetType::Texture3D, false },
            { AssetType::Scene,     true },
            { AssetType::Prefab,    true },
            { AssetType::Mesh,      false },
            { AssetType::Material,  false },
            { AssetType::LuaScript, true },
            { AssetType::Audio,     true },
        };
    };
}