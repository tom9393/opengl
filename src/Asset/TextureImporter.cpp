#include "TextureImporter.h"

namespace Engine {

    Shared<Texture2D> TextureImporter::LoadTexture2D(const AssetMetadata &metadata)
    {
        return Texture2D::Create(metadata.FilePath);
    }

    AssetMetadata TextureImporter::ImportTexture2D(const std::filesystem::path &path)
    {
        AssetMetadata metadata;
        metadata.Handle = UUID();
        metadata.FilePath = path.string();
        metadata.Name = path.stem().string();
        metadata.Type = AssetType::Texture2D;

        return metadata;
    }
}
