#include "AssetPreviewManager.h"
#include "Asset/AssetManager.h"
#include "EditorIcons.h"
#include "MaterialPreview.h"
#include "PrefabPreview.h"
#include "Render/Passes/ShadowPass2D.h"
#include "Render/Passes/ShadowPass3D.h"
#include "Render/Passes/GeometryPass.h"
#include "Render/Passes/ScenePass.h"
#include "Render/Passes/PostProcessPass.h"
#include "Texture2DPreview.h"

namespace Engine {

    std::unordered_map<UUID, Shared<Preview>> AssetPreviewManager::s_Previews;

    Unique<RenderGraph> AssetPreviewManager::s_PreviewGraph;
    Shared<DynamicBufferPool> AssetPreviewManager::s_PreviewPool;

    void AssetPreviewManager::Init() {
        s_PreviewGraph = MakeUnique<RenderGraph>(256, 256);
        s_PreviewGraph->AddPass<ShadowPass2D>("ShadowPass2D");
        s_PreviewGraph->AddPass<ShadowPass3D>("PointShadowPass");
        s_PreviewGraph->AddPass<GeometryPass>("GeometryPass");
        s_PreviewGraph->AddPass<ScenePass>("ScenePass");
        // No ParticlePass in the preview graph, so feed post directly from SceneColor.
        s_PreviewGraph->AddPass<PostProcessPass>("PostProcessPass", "SceneColor");

        s_PreviewPool = MakeShared<DynamicBufferPool>(1024 * 1024);
    }

    void AssetPreviewManager::Shutdown() {
        s_Previews.clear();
    }

    uint32_t AssetPreviewManager::GetPreview(UUID assetHandle) {
        AssetMetadata metadata = AssetManager::GetMetadata(assetHandle);
        if (metadata.Type == AssetType::LuaScript) return EditorIcons::Lua()->GetID();
        if (metadata.Type == AssetType::Audio) return EditorIcons::Audio()->GetID();

        if (s_Previews.find(assetHandle) != s_Previews.end()) {
            uint32_t texID = s_Previews[assetHandle]->GetCachedTextureID();
            return texID != 0 ? texID : EditorIcons::File()->GetID();
        } else {
            AssetManager::GetAssetAsync(assetHandle, [&](Shared<Asset> asset){

                switch (metadata.Type) {
                    case AssetType::Scene:
                    case AssetType::Prefab: {
                        Shared<PrefabPreview> prev = MakeShared<PrefabPreview>();
                        prev->SetAsset(asset);
                        prev->UpdatePreview(*s_PreviewGraph, s_PreviewPool.get());
                        s_Previews[assetHandle] = prev;
                        break;
                    }
                    case AssetType::Material: {
                        Shared<MaterialPreview> prev = MakeShared<MaterialPreview>();
                        prev->SetAsset(asset);
                        prev->UpdatePreview(*s_PreviewGraph, s_PreviewPool.get());
                        s_Previews[assetHandle] = prev;
                        break;
                    }
                    case AssetType::Texture2D: {
                        Shared<Texture2DPreview> prev = MakeShared<Texture2DPreview>();
                        prev->SetAsset(asset);
                        prev->UpdatePreview(*s_PreviewGraph, s_PreviewPool.get());
                        s_Previews[assetHandle] = prev;
                        break;
                    }
                }
            });
        }

        return EditorIcons::File()->GetID();
    }

    void AssetPreviewManager::UpdatePreview(UUID assetHandle) {
        if (s_Previews.find(assetHandle) != s_Previews.end()) {
            s_Previews[assetHandle]->UpdatePreview(*s_PreviewGraph, s_PreviewPool.get());
        }
    }

}