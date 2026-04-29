#include "AssetPreviewManager.h"
#include "../Asset/AssetManager.h"
#include "PrefabPreview.h"
#include "MaterialPreview.h"

namespace Engine {

    std::unordered_map<UUID, Shared<Preview>> AssetPreviewManager::s_Previews;
    Shared<Texture2D> AssetPreviewManager::s_DefaultFileIcon;
    Shared<Texture2D> AssetPreviewManager::s_DefaultFolderIcon;
    Shared<Texture2D> AssetPreviewManager::s_LuaIcon;

    void AssetPreviewManager::Init() {
		s_DefaultFileIcon = Texture2D::Create("../assets/icons/documentIcon.png");
		s_DefaultFolderIcon = Texture2D::Create("../assets/icons/folderIcon.png");
		s_LuaIcon = Texture2D::Create("../assets/icons/luaIcon.png");
    }

    void AssetPreviewManager::Shutdown() {
        s_Previews.clear();
        s_DefaultFileIcon.reset();
        s_DefaultFolderIcon.reset();
    }

    uint32_t AssetPreviewManager::GetPreview(UUID assetHandle) {
        if (s_Previews.find(assetHandle) != s_Previews.end()) {
            return s_Previews[assetHandle]->GetPreview();
        }

        Shared<Preview> previewTexture = GeneratePreview(assetHandle);
        if (previewTexture) {
            s_Previews[assetHandle] = previewTexture;
            return previewTexture->GetPreview();
        }

        return s_DefaultFileIcon->GetID();
    }

    void AssetPreviewManager::UpdatePreview(UUID assetHandle) {
        if (s_Previews.find(assetHandle) == s_Previews.end())
        {
            Shared<Preview> previewTexture = GeneratePreview(assetHandle);
            if (previewTexture) {
                s_Previews[assetHandle] = previewTexture;
            }
        }
        const AssetMetadata& metadata = AssetManager::GetMetadata(assetHandle);
        switch (metadata.Type)
        {
            case AssetType::Prefab: {
                Shared<Asset> asset = AssetManager::GetAsset<Prefab>(assetHandle);
                s_Previews[assetHandle]->UpdatePreview();
                break;
            }
            case AssetType::Material: {
                Shared<Asset> asset = AssetManager::GetAsset<Material>(assetHandle);
                s_Previews[assetHandle]->UpdatePreview();
                break;
            }
        }
    }

    Shared<Preview> AssetPreviewManager::GeneratePreview(UUID assetHandle) {
        const AssetMetadata& metadata = AssetManager::GetMetadata(assetHandle);
        
        switch (metadata.Type) {
            case AssetType::Prefab: {
                Shared<Asset> prefab = AssetManager::GetAsset<Prefab>(assetHandle);
                Shared<PrefabPreview> prev = MakeShared<PrefabPreview>();
                prev->SetAsset(prefab);
                prev->UpdatePreview();
                return prev;
            }
            case AssetType::Material: {
                Shared<Asset> material = AssetManager::GetAsset<Material>(assetHandle);
                Shared<MaterialPreview> prev = MakeShared<MaterialPreview>();
                prev->SetAsset(material);
                prev->UpdatePreview();
                return prev;
            }
        }
        return nullptr;
    }

}