#pragma once

#include "../../pch.h"
#include "Core/UUID.h"
#include "Asset/Texture.h"
#include "Render/RenderGraph.h"

namespace Engine {

    class Scene;
    class Material;

    class AssetPreviewManager {
    public:
        static void Init();
        static void Shutdown();

        static uint32_t GetPreview(UUID assetHandle);

        static void UpdatePreview(UUID assetHandle);

    private:
        static Shared<Texture2D> GeneratePreview(UUID assetHandle);
        static Shared<Texture2D> GenerateModelPreview(Shared<Scene> &model);
        static Shared<Texture2D> GenerateMaterialPreview(Shared<Material> &material);

    private:
        static std::unordered_map<UUID, Shared<Texture2D>> s_Previews;

        static RenderGraph s_RenderGraph;
        static Shared<DynamicBufferPool> s_InstancePool;
        
        static Shared<Texture2D> s_DefaultFileIcon;
        static Shared<Texture2D> s_DefaultFolderIcon;
        static Shared<Texture2D> s_LuaIcon;
    };

}