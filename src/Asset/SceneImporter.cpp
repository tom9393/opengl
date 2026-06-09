#include "Core/Application.h"
#include "AssetUploadQueue.h"
#include "SceneImporter.h"
#include "Asset/AssimpLoader.h"
#include "Asset/SceneBuilder.h"
#include "Asset/PmSceneSerializer.h"

namespace Engine {

    Shared<Scene> SceneImporter::LoadScene(const AssetMetadata &metadata)
    {
        std::string ext = std::filesystem::path(metadata.FilePath).extension().string();

        if (ext == ".pmblend")
        {
            Shared<Scene> scene = MakeShared<Scene>();
            SceneSerializer serializer(scene);
            serializer.Deserialize(metadata.FilePath);
            return scene;
        }

        if (ext == ".pmscene")
        {
            std::optional<ParsedScene> parsed = PmSceneSerializer::Deserialize(metadata.FilePath);
            if (!parsed)
                return nullptr;
            return SceneBuilder::BuildPrefab(metadata, *parsed);
        }

        std::optional<ParsedScene> parsed = AssimpLoader::LoadScene(metadata);
        if (!parsed)
            return nullptr;

        return SceneBuilder::BuildPrefab(metadata, *parsed);
    }

    void SceneImporter::LoadSceneAsync(const AssetMetadata &metadata, std::function<void(Shared<Asset>)> cb)
    {
        std::string ext = std::filesystem::path(metadata.FilePath).extension().string();

        if (ext == ".pmblend")
        {
            Application::Get().GetThreadPool().Enqueue([metadata, cb]() {
                Shared<Scene> scene = MakeShared<Scene>();
                SceneSerializer serializer(scene);
                serializer.Deserialize(metadata.FilePath);
                AssetUploadQueue::Submit([metadata, scene, cb]() {
                    if (cb) cb(scene);
                });
            });
            return;
        }

        if (ext == ".pmscene")
        {
            Application::Get().GetThreadPool().Enqueue([metadata, cb]() {
                std::optional<ParsedScene> parsed = PmSceneSerializer::Deserialize(metadata.FilePath);
                if (!parsed)
                    return;

                AssetUploadQueue::Submit([metadata, parsed = std::move(*parsed), cb]() {
                    Shared<Prefab> prefab = SceneBuilder::BuildPrefab(metadata, parsed);
                    if (cb) cb(prefab);
                });
            });
            return;
        }

        Application::Get().GetThreadPool().Enqueue([metadata, cb]() {
            std::optional<ParsedScene> parsed = AssimpLoader::LoadScene(metadata);
            if (!parsed)
                return;

            AssetUploadQueue::Submit([metadata, parsed = std::move(*parsed), cb]() {
                Shared<Prefab> prefab = SceneBuilder::BuildPrefab(metadata, parsed);
                if (cb) cb(prefab);
            });
        });
    }

    Shared<AssetMetadata> SceneImporter::ImportScene(const std::filesystem::path &path)
    {
        Shared<AssetMetadata> metadata = MakeShared<AssetMetadata>();
        metadata->Handle = UUID();
        metadata->FilePath = path.string();
        metadata->Name = path.stem().string();
        metadata->Type = AssetType::Scene;

        return metadata;
    }
}
