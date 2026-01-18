#include "TextureImporter.h"

namespace Engine {

    Shared<Texture2D> TextureImporter::LoadTexture2D(const std::filesystem::path &path)
    {
        return Texture2D::Create(path);
    }
}
