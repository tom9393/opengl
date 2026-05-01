#pragma once

#include "../../pch.h"
#include "Core/UUID.h"
#include "Asset/Texture.h"
#include "Preview.h"

namespace Engine {

    class Prefab;
    class Material;

    class AssetPreviewManager {
    public:
        static void Init();
        static void Shutdown();

        static uint32_t GetPreview(UUID assetHandle);

        static void UpdatePreview(UUID assetHandle);

    private:
        static Shared<Preview> GeneratePreview(UUID assetHandle);

    private:
        static std::unordered_map<UUID, Shared<Preview>> s_Previews;

        static Shared<Texture2D> s_DefaultFileIcon;
        static Shared<Texture2D> s_DefaultFolderIcon;
        static Shared<Texture2D> s_LuaIcon;
        static Shared<Texture2D> s_ModelIcon;
    };

}