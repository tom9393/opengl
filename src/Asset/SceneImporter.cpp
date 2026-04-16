#include "SceneImporter.h"
#include "../Asset/AssimpLoader.h"

namespace Engine {

    Shared<Scene> SceneImporter::LoadScene(const AssetMetadata &metadata)
    {
        Shared<Scene> scene = MakeShared<Scene>();
        SceneSerializer serializer(scene);
        serializer.Deserialize(metadata.FilePath);
        return scene;
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
